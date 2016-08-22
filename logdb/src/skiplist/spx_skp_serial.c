/************************************************************************
  > File Name: spx_skp_serial.c
  > Author: shuaixiang
  > Mail: shuaixiang@yuewen.com
  > Created Time: Tue 15 Sep 2015 02:37:28 AM UTC
 ************************************************************************/
#include "spx_skp_serial.h" 
#include "spx_message.h"
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#define SpxSkpSerialMaxLogSize 10485760//the max size of data file 
#define SpxSkpSerialBaseSize 20//count(long) + offset(long) + version(int)
#define SpxSkpSerialHeadSize 10485760//256M 8192 for test
#define SpxSkpSerialItemSize 13 //Head Item size: isExist(char) + value_item_len(int) + valueItemOffset(long)
//#define SpxSkpSerialTypeSize 2 //keyType: char + valueType: char  
#define SpxSkpSerialKVLen 8 // keyLen: int + valueLen: int
#define SpxSkpMaxLogCount 1024
#define SpxSkpSerialPathLen 255
#define SpxSkpSerialFileNameSize 255 
#define SpxSkpSerialMetadataFileSize 50 
#define SpxSkpIndexConf "./skiplist/config/index.config" 

static int g_file_count = 0; //single thread safe 
static char * cur_file = NULL;

//declare
static int64_t spx_skp_serial_get_file_size(const char * file);
static int spx_skp_serial_expand_file(const char * file , int64_t size);
static int spx_skp_serial_flush_map(struct spx_skp_serial_mf *mf);
static struct spx_skp_serial_mf *spx_skp_serial_map(const char * file, int64_t size, int64_t offset);
static int spx_skp_serial_un_map(struct spx_skp_serial_mf * mf);
static struct spx_skp_serial_metadata *spx_skp_serial_write_data(char *file, const ubyte_t *data, size_t len);
static char * spx_skp_serial_which_file();
static ubyte_t * spx_skp_serial_m2b(struct spx_skp_serial_metadata *md, int *byteLen);
static struct spx_skp_serial_metadata *spx_skp_serial_b2m(ubyte_t *b);
static char * gen_local_time();
static int spx_skp_serial_is_dir_exist(const char *dir_path);
static struct spx_skp_serial_mf * spx_skp_serial_mf_list_find(const char *path);
static int spx_skp_serial_mf_list_insert(const char * path, struct spx_skp_serial_mf *mf);
static int64_t spx_skp_serial_index_insert(const char *file, ubyte_t * head, int keyLen, void * key, int valueLen, void * value );
static struct spx_skp_serial_bin_kv_node * spx_skp_serial_index_visit(ubyte_t * i_map, const char * file);
static int spx_skp_serial_index_delete(ubyte_t * head, int64_t index);
static struct spx_skp_serial_mf* spx_skp_serial_init(const char *file);
static int spx_skp_serial_bin_kv_list_free(struct spx_skp_serial_bin_kv_node *byte_kv_list);
static void spx_skp_get_idx_path(char *skp_name, char *path, int size);

struct spx_skp_serial_bin_kv_node{
    int key_len;
    ubyte_t *key;//binary key
    int value_len;
    ubyte_t *value;//binary value
    int index;
    struct spx_skp_serial_bin_kv_node *next_kv_node;
};

struct spx_skp_serial_mf_list_node{
    char path[SpxSkpSerialPathLen];
    struct spx_skp_serial_mf *mf;
    struct spx_skp_serial_mf_list_node * next_mf;
};

struct spx_skp_serial_mf_list{
    struct spx_skp_serial_mf_list_node *head;
    struct spx_skp_serial_mf_list_node *tail;
} spx_skp_serial_mf_list_head;


static int spx_skp_serial_bin_kv_list_free(struct spx_skp_serial_bin_kv_node *byte_kv_list){/*{{{*/
    if (NULL == byte_kv_list){
        printf("byte_kv_list is NULL\n");
        return -1;
    }

    struct spx_skp_serial_bin_kv_node * cur_kv_node = byte_kv_list;

    while (cur_kv_node != NULL){
        struct spx_skp_serial_bin_kv_node * free_kv_node = cur_kv_node;
        cur_kv_node = cur_kv_node->next_kv_node;
        free(free_kv_node->key);
        free(free_kv_node->value);
        free(free_kv_node);
    }

    return 0;
}/*}}}*/

static int spx_skp_serial_mf_list_insert(const char * path, struct spx_skp_serial_mf *mf){/*{{{*/
    if (NULL == path || NULL == mf){
        printf("path or spx_skp_serial_mf is NULL\n");
        return -1;
    }

    struct spx_skp_serial_mf_list_node *new_mf = (struct spx_skp_serial_mf_list_node *) malloc(sizeof(struct spx_skp_serial_mf_list_node));
    new_mf->mf = mf;
    strncpy(new_mf->path, path, strlen(path)); 
    *(new_mf->path + strlen(path)) = '\0';
    new_mf->next_mf = NULL;

    if (NULL == spx_skp_serial_mf_list_head.head){
        spx_skp_serial_mf_list_head.head = new_mf;
        spx_skp_serial_mf_list_head.tail = new_mf;
    } else if (spx_skp_serial_mf_list_head.tail != NULL){
        spx_skp_serial_mf_list_head.tail->next_mf = new_mf;
        spx_skp_serial_mf_list_head.tail = new_mf;
    } else {
        printf("mf list tail is NULL\n");
        return -1;
    }

    return 0;
}/*}}}*/

static struct spx_skp_serial_mf * spx_skp_serial_mf_list_find(const char *path){/*{{{*/
    struct spx_skp_serial_mf_list_node *tmp_mf = spx_skp_serial_mf_list_head.head;
    while (NULL != tmp_mf){
        if (!strcmp(tmp_mf->path, path)){
            return tmp_mf->mf;
        } 

        tmp_mf = tmp_mf->next_mf;
    }

    return NULL;
}/*}}}*/

static int spx_skp_serial_is_dir_exist(const char *dir_path){/*{{{*/
    if (NULL == dir_path)
        return -1;
    if (-1 == access(dir_path, F_OK)){
        printf("%s is not exist, will make it\n", dir_path);
        if (-1 == mkdir(dir_path, S_IRWXU | S_IRWXG | S_IROTH | S_IROTH))
            return -1;
    }
    return 0;
}/*}}}*/

static int spx_skp_serial_build_time_dir(char *path){/*{{{*/
    time_t now;
    struct tm *p;
    time(&now);
    p = localtime(&now);
    char dir_path[SpxSkpSerialFileNameSize];
    char dir_tmp0[SpxSkpSerialFileNameSize];
    char dir_tmp1[SpxSkpSerialFileNameSize];
    char dir_tmp2[SpxSkpSerialFileNameSize];
    char dir_tmp3[SpxSkpSerialFileNameSize];
    getcwd(dir_path, sizeof(dir_path));
    snprintf(dir_tmp0, sizeof(dir_tmp0), "%s/skiplist/data", dir_path);

    snprintf(dir_tmp1, sizeof(dir_tmp1), "%s/%d", dir_tmp0, 1900 + p->tm_year);
    if (-1 == spx_skp_serial_is_dir_exist(dir_tmp1)){
        printf("mkdir %s failed", dir_tmp1);
        return -1;
    }
    snprintf(dir_tmp2, sizeof(dir_tmp2), "%s/%02d", dir_tmp1, p->tm_mon + 1);
    if (-1 == spx_skp_serial_is_dir_exist(dir_tmp2)){
        printf("mkdir %s failed", dir_tmp2);
        return -1;
    }
    snprintf(dir_tmp3, sizeof(dir_tmp3), "%s/%02d", dir_tmp2, p->tm_mday);
    if (-1 == spx_skp_serial_is_dir_exist(dir_tmp3)){
        printf("mkdir %s failed", dir_tmp3);
        return -1;
    }

    snprintf(path, SpxSkpSerialFileNameSize, "%d/%02d/%02d", 1900 + p->tm_year, p->tm_mon + 1, p->tm_mday);

    return 0;
}/*}}}*/

static void spx_skp_get_idx_path(char *skp_name, char *path, int size){/*{{{*/
    char dir[SpxSkpSerialPathLen];
    getcwd(dir, sizeof(dir));
    snprintf(path, size, "%s/skiplist/index/%s.idx", dir, skp_name); 
}/*}}}*/

/*
* Mapped init
*/
//get FileSize
static int64_t spx_skp_serial_get_file_size(const char * file){/*{{{*/
    struct stat sb;
    if ( stat(file, &sb) == -1 ){
        perror("stat");
        return 0;
    }

    return sb.st_size;
}/*}}}*/

//increase FileSize
static int spx_skp_serial_expand_file(const char * file , int64_t size){/*{{{*/
    int fd;
    int64_t cur_size = spx_skp_serial_get_file_size(file);

    if ((fd = open(file, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR)) < 0){
        perror("open");
        return -1;
    }

    ftruncate(fd, cur_size + size);
    close(fd);

    return 0;
}/*}}}*/

//flush map
static int spx_skp_serial_flush_map(struct spx_skp_serial_mf *mf){/*{{{*/

    if ((msync((void *) (mf->mapped - mf->distance), (mf->size + mf->distance), MS_SYNC)) == -1){
        return -1;
    }

    return 0;
}/*}}}*/

//unMap
static int spx_skp_serial_un_map(struct spx_skp_serial_mf * mf){/*{{{*/
    if ((munmap((void *)(mf->mapped - mf->distance), (mf->size + mf->distance))) == -1){
        printf("munmap file failed\n");
        free(mf);
        return -1;
    }

    free(mf);
    return 0;
}/*}}}*/

//map 
static struct spx_skp_serial_mf *spx_skp_serial_map(const char * file, int64_t size, int64_t offset){/*{{{*/
    int fd;
    int64_t file_size = spx_skp_serial_get_file_size(file);
    int page_size = sysconf(_SC_PAGE_SIZE);
    int expand_size = size;
    int i = 1;
    ubyte_t *mapped;
    struct spx_skp_serial_mf *mf = (struct spx_skp_serial_mf*) malloc(sizeof(struct spx_skp_serial_mf));

    while ((file_size - offset) < size){
        spx_skp_serial_expand_file(file, expand_size);
        file_size = spx_skp_serial_get_file_size(file);
        expand_size = page_size * i++;
    }

    int align_offset = (offset/page_size)*page_size;
    int distance = offset - align_offset;

    if ((fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) < 0){
        perror("open");
        return NULL;
    }

    if ((mapped = (ubyte_t*) mmap(NULL, distance + size, PROT_READ | 
                    PROT_WRITE, MAP_SHARED, fd, align_offset)) == (void *) -1){
        perror("mmap");
    }

    close(fd);

    mf->mapped = mapped + distance;
    mf->size = size;
    mf->distance = distance;

    return mf;

}/*}}}*/

/*
 * serial skiplist
 */

//read config and init skp_name
struct spx_skp_idx *spx_skp_read_config()/*{{{*/
{
    if (spx_skp_idx_head.next_idx == NULL){
        FILE *fp = fopen(SpxSkpIndexConf, "r");
        char line[100];

        struct spx_skp_idx *tmp_idx = &spx_skp_idx_head;
        while(fgets(line, sizeof(line), fp)){
            if('#' != *line && '\n' != *line){
                int i = 0;
                for(i = 0; i < (int)strlen(line); i++){
                    if( *(line + i) == ':')
                    break;
                }

                struct spx_skp_idx * new_idx = (struct spx_skp_idx *) malloc(sizeof(struct spx_skp_idx));           
                new_idx->next_idx = NULL;
                new_idx->index = (char *) malloc(sizeof(char) * (i + 1));
                strncpy(new_idx->index, line, i);
                *(new_idx->index + i) = '\0';
                if (!strcmp((line + i + 1), "string\n")){
                    new_idx->type = 0;  
                } else if(!strcmp((line + i + 1), "int\n")){
                    new_idx->type = 1;
                } else if(!strcmp((line + i + 1), "long\n")){
                    new_idx->type = 2;
                } else{
                    new_idx->type = -1;
                }

                tmp_idx->next_idx = new_idx; 
                tmp_idx = new_idx;
            }
        }
    }

    return &spx_skp_idx_head;
}/*}}}*/

struct spx_skp * spx_skp_get(char *skp_name)/*{{{*/
{
    struct spx_skp_list_node *tmp_skp = spx_skp_list_head.head;
    if(NULL == tmp_skp){
        printf("tmp_skpis NULL\n");
        return NULL;
    }

    while (tmp_skp){
        if( tmp_skp->skp && tmp_skp->skp->name && !strncmp(tmp_skp->skp->name, skp_name, strlen(tmp_skp->skp->name)))
        return tmp_skp->skp; 
        tmp_skp= tmp_skp->next_skp; 
    }

    return NULL;
}/*}}}*/

struct spx_skp * spx_skp_add(SpxSkpCmpDelegate cmp_key, SpxSkpCmpDelegate cmp_value, SpxSkpB2ODelegate byte2key, SpxSkpB2ODelegate byte2value, const char * skp_name, SpxSkpFreeDelegate free_key, SpxSkpFreeDelegate free_value)/*{{{*/
{
    struct spx_skp *skp = spx_skp_new(cmp_key, cmp_value, skp_name, free_key, free_value);
    spx_skp_unserial(skp, byte2key, byte2value);
    if (NULL == skp){
        printf("spx_skp_new failed, skp is NULL\n");
        return NULL;
    }
    //spx_skp_print(skp);
    struct spx_skp_list_node *new_skp = (struct spx_skp_list_node *) calloc(1, sizeof(struct spx_skp_list_node));
    new_skp->skp = skp;
    struct spx_skp_list_node *tmp_skp = spx_skp_list_head.tail;

    if (NULL == tmp_skp){
        spx_skp_list_head.head = new_skp;
        spx_skp_list_head.tail = new_skp;
    } else {
        tmp_skp->next_skp = new_skp;
        spx_skp_list_head.tail = new_skp;
    }

    return skp;
}/*}}}*/


//serialzation struct spx_skp
int spx_skp_serial(struct spx_skp *sl, SpxSkpO2BDelegate key2byte, SpxSkpO2BDelegate value2byte){/*{{{*/
    char path[SpxSkpSerialPathLen];
    spx_skp_get_idx_path(sl->name, path, sizeof(path));
    struct spx_skp_serial_mf *mf = spx_skp_serial_init(path);
    ubyte_t * i_map = mf->mapped;
    if(NULL == i_map){
        printf("i_map is NULL\n");
        return -1;
    }

    struct spx_skp_node  *pre_node = sl->head, *cur_node = pre_node->next_node[0];

    while(cur_node){
        if(0 == pre_node->size && pre_node != sl->head)
            spx_skp_node_remove(sl, pre_node); //remove the node which is marked deleted
        int key_len = 0;
        int value_len = 0;
        struct spx_skp_node_value *v = cur_node->value;
        while(v){
            switch(v->status){
                case NEWADD:
                    {
                        ubyte_t *key = key2byte(cur_node->key, &key_len);
                        ubyte_t *value = value2byte(v->value, &value_len);
                        int64_t index = spx_skp_serial_index_insert(path, i_map, key_len, key, value_len, value);  
                        if(index != -1){
                            v->index = index; 
                            v->status = EXISTED;
                        }
                    }
                    break;

                case DELETED:
                    {
                        if(v->index != -1){
                            spx_skp_serial_index_delete(i_map, v->index);
                        }
                    }
                    break;
                default:
                    break;
            }
            v = v->next_value; 
        }

        pre_node = cur_node;
        cur_node = pre_node->next_node[0];
    }

    return 0;
}/*}}}*/

//unserial spx_skp
int spx_skp_unserial(struct spx_skp *sl, SpxSkpB2ODelegate byte2key, SpxSkpB2ODelegate byte2value){/*{{{*/
    char path[SpxSkpSerialPathLen];
    spx_skp_get_idx_path(sl->name, path, sizeof(path));
    struct spx_skp_serial_mf *mf = spx_skp_serial_init(path);
    ubyte_t *i_map = mf->mapped; 
    if(NULL == i_map){
        printf("i_map is NULL\n");
        return -1;
    }

    struct spx_skp_serial_bin_kv_node * byte_kv_list = spx_skp_serial_index_visit(i_map, path);
    while(byte_kv_list->next_kv_node){
        struct spx_skp_serial_bin_kv_node * serial_node = byte_kv_list->next_kv_node;
        //printf("%s %s \n", node->key, node->value);
        struct spx_skp_node *new_sl_node = spx_skp_insert(sl, byte2key(serial_node->key, serial_node->key_len), byte2value(serial_node->value, serial_node->value_len));
        if(NULL == new_sl_node){
            printf("new_sl_node is NULL\n");
            return -1;
        }
        struct spx_skp_node_value *v = new_sl_node->value; 
        while(v->next_value){
            v = v->next_value;
        }
        v->index = serial_node->index;
        v->status = EXISTED;

        byte_kv_list = byte_kv_list->next_kv_node;
    }

    spx_skp_serial_bin_kv_list_free(byte_kv_list);

    return 0;
}/*}}}*/

/*
 *Index Operation
 */

//visit index
static struct spx_skp_serial_bin_kv_node * spx_skp_serial_index_visit(ubyte_t * i_map, const char * file){/*{{{*/
    int64_t count = spx_msg_b2l(i_map);

    struct spx_skp_serial_bin_kv_node *byte_kv_list = (struct spx_skp_serial_bin_kv_node *) malloc(sizeof(struct spx_skp_serial_bin_kv_node));//kv list head
    byte_kv_list->value = NULL;
    byte_kv_list->key =  NULL;
    byte_kv_list->next_kv_node = NULL;
    struct spx_skp_serial_bin_kv_node *cur_kvp = byte_kv_list;

    ubyte_t * start_ptr = i_map + SpxSkpSerialBaseSize;
    printf("version: %d\n", spx_msg_b2i(i_map + 16));

    int i;
    for(i = 0; i< count; i++){
        ubyte_t * curPos = start_ptr + i*SpxSkpSerialItemSize;
        char isExist = *curPos;
        int len = spx_msg_b2i(curPos + 1);
        int64_t offset = spx_msg_b2l(curPos + 5);

        if(isExist == '0')
            continue;
        //printf("#%d: %c %ld %d ->  ", i, isExist, offset, len);

        struct spx_skp_serial_mf *mf = spx_skp_serial_map(file, len, offset);  
        ubyte_t *body_pos = mf->mapped;
        int key_len = spx_msg_b2i(body_pos);
        ubyte_t * key = (ubyte_t*)calloc(sizeof(ubyte_t), key_len);
        memcpy(key, body_pos + 4, key_len);

        int value_len = spx_msg_b2i(body_pos + 4 + key_len);
        ubyte_t * value = (ubyte_t*)calloc(sizeof(ubyte_t), value_len);
        memcpy(value, body_pos + 8 + key_len, value_len);

        //printf("%d %d %s : %d %d %s\n", keyType, keyLen, key, valueType, valueLen, value); 
        struct spx_skp_serial_bin_kv_node *kvp = (struct spx_skp_serial_bin_kv_node *)malloc(sizeof(struct spx_skp_serial_bin_kv_node));
        kvp->key_len = key_len;
        kvp->key = key;
        kvp->value_len = value_len;
        kvp->value = value;
        kvp->next_kv_node = NULL;
        kvp->index = i;

        cur_kvp->next_kv_node = kvp;
        cur_kvp = kvp;

        if ( 0 != spx_skp_serial_un_map(mf) ){
            printf("spx_skp_serial_un_map failed\n");
        }
    }

    return byte_kv_list;
}/*}}}*/

//insert index
static int64_t spx_skp_serial_index_insert(const char *file, ubyte_t * head, int keyLen, void * key, int valueLen, void * value ){/*{{{*/
    int64_t count = spx_msg_b2l(head); //counts of HEAD
    int64_t offset = spx_msg_b2l(head + 8);// offset of new Value
    int version = spx_msg_b2i(head + 16);//vserion of skiplist
    int64_t index = count;

    ubyte_t * start_ptr = head + SpxSkpSerialBaseSize;

    int i = 0;
    for(; i<count; i++){
        if(*(start_ptr + SpxSkpSerialItemSize*i) == '0')
            break;
    }

    //set head
    ubyte_t * head_item_ptr = start_ptr + i*SpxSkpSerialItemSize;
    int  value_item_len = keyLen + valueLen + SpxSkpSerialKVLen;
    spx_msg_i2b(head + 16, ++version);

    if(i < count && (value_item_len <= spx_msg_b2i(head_item_ptr + 1))){
        *head_item_ptr = '1';
        offset = spx_msg_b2l(head_item_ptr + 5);//offset of this value
        index = i;
    }else{
        //init HEAD
        head_item_ptr = start_ptr + count*SpxSkpSerialItemSize;
        *head_item_ptr = '1';//set exist
        spx_msg_i2b(head_item_ptr + 1, value_item_len);//set value_item_len
        spx_msg_l2b(head_item_ptr + 5, offset); //set offset

        spx_msg_l2b(head, ++count);//increase item count in head
        spx_msg_l2b(head + 8, offset + value_item_len);//set new offset in head
    }

    //set body
    struct spx_skp_serial_mf *mf = spx_skp_serial_map(file, spx_msg_b2i(head_item_ptr + 1), offset);
    ubyte_t *body_pos = mf->mapped;

    spx_msg_i2b(body_pos, keyLen);
    memcpy(body_pos + 4, key, keyLen); 

    spx_msg_i2b((body_pos + 4 + keyLen), valueLen);
    memcpy(body_pos + 8 + keyLen, value, valueLen); 

    free(value);
    free(key);
    if (0 != spx_skp_serial_un_map(mf) ){
        printf("spx_skp_serial_un_map failed\n");
    }

    return index;
}/*}}}*/

//delete single index
static int spx_skp_serial_index_delete(ubyte_t * head, int64_t index){/*{{{*/
    if(head == NULL)
        return -1;

    int64_t count = spx_msg_b2l(head);
    int version = spx_msg_b2i(head + 16);

    if( index > count -1)
        return -1;

    ubyte_t * start_ptr = head + SpxSkpSerialBaseSize;
    ubyte_t * head_item_ptr = start_ptr + index*SpxSkpSerialItemSize;

    if(*head_item_ptr == '1')
        *head_item_ptr = '0';
    else
        return -1;

    spx_msg_i2b(head + 16, ++version);

    return 0;
}/*}}}*/

static struct spx_skp_serial_mf* spx_skp_serial_init(const char *file){/*{{{*/
    if (NULL == file){
        printf("fiel to init is NULL\n");
        return NULL;
    }

    struct spx_skp_serial_mf *mf = spx_skp_serial_mf_list_find(file);

    if (NULL != mf)
        return mf;

    if (access(file, 0)){
        if(spx_skp_serial_expand_file(file, SpxSkpSerialHeadSize) == -1)
            return NULL;

        struct spx_skp_serial_mf *mf = spx_skp_serial_map(file, SpxSkpSerialBaseSize, 0); 
        ubyte_t * mp = mf->mapped; 
        spx_msg_l2b(mp, 0);
        spx_msg_l2b(mp + 8, SpxSkpSerialHeadSize);
        spx_msg_i2b(mp + 16, 0);
        if ( 0 != spx_skp_serial_un_map(mf) ){
            printf("spx_skp_serial_un_map failed\n");
        }
    }

    mf = spx_skp_serial_map(file, SpxSkpSerialHeadSize, 0);
    spx_skp_serial_mf_list_insert(file, mf);

    return mf;
}/*}}}*/

/*
 *Data Operation
 */

struct spx_skp_serial_md_list * spx_skp_serial_md_list_new(){/*{{{*/
    struct spx_skp_serial_md_list *md_list = (struct spx_skp_serial_md_list *) malloc(sizeof(*md_list));    
    if (NULL == md_list){
        printf("malloc spx_skp_serial_md_list failed\n");
        return NULL;
    }


    md_list->head = NULL;
    md_list->tail = NULL;

    return md_list;
}/*}}}*/

int spx_skp_serial_md_list_insert(struct spx_skp_serial_md_list * md_list, struct spx_skp_serial_metadata *md){/*{{{*/
    if (NULL == md_list){
        printf("md_list is NULL\n");
        return -1;
    }

    if (NULL == md){
        printf("md is NULL\n");
        return -1;
    }

    struct spx_skp_serial_md_list_node * new_md_node = (struct spx_skp_serial_md_list_node *) malloc(sizeof(*new_md_node));
    if (NULL == new_md_node){
        printf("malloc spx_skp_serial_md_list_node failed\n");
        return -1;
    }

    new_md_node->md = md;
    new_md_node->next_md = NULL;

    if (NULL == md_list->tail){
        md_list->head = new_md_node;
        md_list->tail = new_md_node;
    } else {
        md_list->tail->next_md = new_md_node;
        md_list->tail = new_md_node;
    }

    return 0;
}/*}}}*/

int spx_skp_serial_md_list_free(struct spx_skp_serial_md_list * md_list){/*{{{*/
    if (NULL == md_list){
        printf("md_list is NULL\n");
        return -1;
    }

    struct spx_skp_serial_md_list_node * tmp_md = md_list->head;

    while(tmp_md){
        struct spx_skp_serial_md_list_node * spx_skp_serial_free_md = tmp_md;
        tmp_md = tmp_md->next_md;
        free(spx_skp_serial_free_md);
        //attention md can not be free, it is on skiplist
    }

    free(md_list);

    return 0;
}/*}}}*/

static struct spx_skp_serial_metadata *spx_skp_serial_write_data(char *file, const ubyte_t *data, size_t len){/*{{{*/
    int fd;
    char path[SpxSkpSerialFileNameSize];
    char path1[SpxSkpSerialFileNameSize];
    getcwd(path, sizeof(path));
    snprintf(path1, sizeof(path1), "%s/skiplist/data/%s", path, file);

    FILE *fp = fopen(path1, "a+"); 
    if (NULL == fp){
        perror("fp is NULL");
        return NULL;
    }
    fseek(fp, 0, SEEK_END);    

    struct spx_skp_serial_metadata *md= (struct spx_skp_serial_metadata*) malloc(sizeof(struct spx_skp_serial_metadata));
    md->file = file;
    md->len = len;
    md->off = ftell(fp);
    md->filename_len = strlen(file);

    fwrite(data, sizeof(char), len, fp);

    fclose(fp);

    return md;
}/*}}}*/

static char * spx_skp_serial_which_file(){/*{{{*/
    char *file = (char*)calloc(1, sizeof(char) * SpxSkpSerialMetadataFileSize);// yyyy/mm/dd\n
    char path[SpxSkpSerialFileNameSize];
    char path1[SpxSkpSerialFileNameSize];
    char path2[SpxSkpSerialFileNameSize];
    char time_path[SpxSkpSerialFileNameSize];
    getcwd(path, sizeof(path));
    if (-1 == spx_skp_serial_build_time_dir(time_path))
        return NULL;
    snprintf(path1, sizeof(path1), "%s/skiplist/data/%s", path, time_path);
    snprintf(path2, sizeof(path2), "%s/%04d.log", path1, g_file_count);

    while(spx_skp_serial_get_file_size(path2) >= SpxSkpSerialMaxLogSize && g_file_count < SpxSkpMaxLogCount)
        snprintf(path2, sizeof(path2), "%s/%04d.log", path1, ++g_file_count);

    if (g_file_count >= SpxSkpMaxLogCount){
        printf("file_count is full\n");
        return NULL;
    }

    snprintf(file, SpxSkpSerialMetadataFileSize, "%s/%04d.log", time_path, g_file_count);
    if ( cur_file != NULL && !strcmp(file, cur_file) ){
        free(file);
        return cur_file;
    } 
                                          
    cur_file = file;//wild pointer
    return file;
}/*}}}*/

static ubyte_t * spx_skp_serial_m2b(struct spx_skp_serial_metadata *md, int *byteLen){/*{{{*/
    int size  = 1 + md->filename_len + 8 + 8; 
    ubyte_t * b = (ubyte_t *)calloc(1, sizeof(ubyte_t)*size);    
    *b = (md->filename_len & 0xff);
    memcpy(b + 1, md->file, md->filename_len);
    spx_msg_l2b((b + 1 + md->filename_len), md->off);
    spx_msg_l2b((b + 1 + md->filename_len + 8), md->len);

    if (NULL != byteLen)
        *byteLen = size;
    return b;
}/*}}}*/

static struct spx_skp_serial_metadata *spx_skp_serial_b2m(ubyte_t *b){/*{{{*/
    struct spx_skp_serial_metadata *md= (struct spx_skp_serial_metadata*) calloc(1, sizeof(struct spx_skp_serial_metadata));
    if(NULL == md){
        printf("malloc struct spx_skp_serial_metadata failed");
        return NULL;
    }

    char filename_len = *b; 
    char *file = (char *)calloc(1, sizeof(char)*(filename_len + 1));
    memcpy(file, (b + 1), filename_len);
    int64_t off = spx_msg_b2l(b + 1 + filename_len);
    int64_t len = spx_msg_b2l(b + 1 + filename_len + 8);

    md->len = len;
    md->off = off;
    md->file = file;
    md->filename_len = filename_len;

    return md;
}/*}}}*/

ubyte_t * spx_skp_serial_data_writer2byte(const ubyte_t *data, size_t len){/*{{{*/
    char * file = spx_skp_serial_which_file();     
    struct spx_skp_serial_metadata *md = spx_skp_serial_write_data(file, data, len);
    if(NULL == md){
        printf("write data error\n");
        return NULL;
    }
    ubyte_t *buf = spx_skp_serial_m2b(md, NULL);

    free(file);
    free(md);

    return buf;
}/*}}}*/

struct spx_skp_serial_metadata* spx_skp_serial_data_writer2md(const ubyte_t *data, size_t len){/*{{{*/
    char * file = spx_skp_serial_which_file();     
    struct spx_skp_serial_metadata *md = spx_skp_serial_write_data(file, data, len);
    if(NULL == md){
        printf("write data error\n");
        return NULL;
    }

    return md;
}/*}}}*/

void spx_skp_serial_free_md(struct spx_skp_serial_metadata *md){/*{{{*/
    free(md->file);
    free(md);
}/*}}}*/

//read data
ubyte_t * spx_skp_serial_data_reader(struct spx_skp_serial_metadata *md){/*{{{*/
    char filename_len = md->filename_len;
    char *tmp_file = md->file;
    int64_t off = md->off;
    int64_t len = md->len;

    char dir_path[SpxSkpSerialFileNameSize];
    char file[SpxSkpSerialFileNameSize];
    getcwd(dir_path, sizeof(dir_path));
    snprintf(file, sizeof(file), "%s/skiplist/data/%s", dir_path, tmp_file);

    FILE *fp = fopen(file, "r+");
    if (NULL == fp){
        perror("fp is NULL");
        return NULL;
    }


    fseek(fp, off, SEEK_SET);
    ubyte_t * data = (ubyte_t *) malloc(sizeof(ubyte_t) * len);
    fread(data, sizeof(ubyte_t), len, fp);

    fclose(fp);
    return data;
}/*}}}*/

// spx_skp_serial_gen_unid
void spx_skp_serial_gen_unid(struct spx_skp_serial_metadata *md, char *unid, size_t size){/*{{{*/
    char fid[100]={0};
    int index = 0;
    char *file = md->file; 
    int64_t off = md->off;

    char *p = file;
    while( *p != '\0'){
        if ( *p >= '0' && *p <= '9')
            fid[index++] = *p;    
        p++;
    }

    snprintf(unid, size, "%s%04ld", fid, off);
}/*}}}*/

/*
 * byte transform
 */

//int
ubyte_t *spx_skp_serial_int2byte(const void * i, int *byte_len){/*{{{*/
    int b = *(int *)i;
    *byte_len = 4;
    ubyte_t *a = (ubyte_t *) malloc(sizeof(ubyte_t) * 4);
    *a = (b>>24) & 0xFF;
    *(a + 1) = (b>>16) & 0xFF;
    *(a + 2) = (b>>8)  & 0xFF;
    *(a + 3) = b & 0xFF;

    return a;
}/*}}}*/

void *spx_skp_serial_byte2int(ubyte_t *b, int obj_len){/*{{{*/
    int *a = (int *) malloc(sizeof(int));
    *a = (((*b)<<24) | ((*(b + 1))<<16) | ((*(b + 2))<<8) | *(b + 3));
    return a;
}/*}}}*/

//int64_t
ubyte_t *spx_skp_serial_long2byte(const void * i, int *byte_len){/*{{{*/
    int64_t n = *(int64_t *)i; 
    *byte_len = 8;
    ubyte_t *b = (ubyte_t *) malloc(sizeof(ubyte_t) * 8);//only for 64bit
    *b = (n >> 56) & 0xFF;
    *(b + 1) = (n >> 48) & 0xFF;
    *(b + 2) = (n >> 40) & 0xFF;
    *(b + 3) = (n >> 32) & 0xFF;
    *(b + 4) = (n >> 24) & 0xFF;
    *(b + 5) = (n >> 16) & 0xFF;
    *(b + 6) = (n >> 8) & 0xFF;
    *(b + 7) = n & 0xFF;

    return b;
}/*}}}*/

void *spx_skp_serial_byte2long(ubyte_t *b, int obj_len){/*{{{*/
    int64_t *n = (int64_t *) malloc(sizeof(int64_t));
    *n =  (((int64_t) (*b)) << 56)
        | (((int64_t) (*(b+1))) << 48)
        | (((int64_t) (*(b + 2))) << 40)
        | (((int64_t) (*(b + 3))) << 32)
        | (((int64_t) (*(b + 4))) << 24)
        | (((int64_t) (*(b + 5))) << 16)
        | (((int64_t) (*(b + 6))) << 8)
        | ((int64_t) (*(b + 7)));

    return n;
}/*}}}*/

//float
ubyte_t *spx_skp_serial_float2byte(const void * i, int *byte_len){/*{{{*/
    union {
        float f; uint32_t t;
    } fu;
    fu.f = *(float *)i; 
    *byte_len = 4;
    ubyte_t *b = (ubyte_t *) malloc(sizeof(ubyte_t) * 4);
    *b = ((ubyte_t *)&fu.t)[3];
    *(b + 1) = ((ubyte_t *)&fu.t)[2];
    *(b + 2) = ((ubyte_t *)&fu.t)[1];
    *(b + 3) = ((ubyte_t *)&fu.t)[0];

    return b;
}/*}}}*/

void *spx_skp_serial_byte2float(ubyte_t *b, int obj_len){/*{{{*/
    union{
        float f; ubyte_t f_buf[4];
    } fu;

    int i = 0 ;
    for( i = 0; i < 4; i++){
        fu.f_buf[i] = *(b + 3 -i);
    }

    float * pf = (float *) malloc(sizeof(float));
    *pf = fu.f;

    return pf;
}/*}}}*/

//double
ubyte_t *spx_skp_serial_double2byte(const void * i, int *byte_len){/*{{{*/
    union {
        double d; uint64_t t;
    } du;
    du.d = *(double *)i; 
    *byte_len = 8;
    ubyte_t *b = (ubyte_t *) malloc(sizeof(ubyte_t) * 8);
    *b = ((ubyte_t *)&du.t)[7];
    *(b + 1) = ((ubyte_t *)&du.t)[6];
    *(b + 2) = ((ubyte_t *)&du.t)[5];
    *(b + 3) = ((ubyte_t *)&du.t)[4];
    *(b + 4) = ((ubyte_t *)&du.t)[3];
    *(b + 5) = ((ubyte_t *)&du.t)[2];
    *(b + 6) = ((ubyte_t *)&du.t)[1];
    *(b + 7) = ((ubyte_t *)&du.t)[0];

    return b;
}/*}}}*/

void *spx_skp_serial_byte2double(ubyte_t *b, int obj_len){/*{{{*/
    union{
        double d; ubyte_t d_buf[8];
    } du;

    int i = 0 ;
    for( i = 0; i < 8; i++){
        du.d_buf[i] = *(b + 7 -i);
    }

    double * pd = (double *) malloc(sizeof(double));
    *pd = du.d;

    return pd;
}/*}}}*/

//struct spx_skp_serial_metadata
ubyte_t * spx_skp_serial_md2byte(const void * i, int *byte_len){/*{{{*/
    struct spx_skp_serial_metadata *md = (struct spx_skp_serial_metadata *)i; 
    return spx_skp_serial_m2b(md, byte_len);
}/*}}}*/

void *spx_skp_serial_byte2md(ubyte_t *b, int obj_len){/*{{{*/
    return spx_skp_serial_b2m(b);
}/*}}}*/

//string
ubyte_t * spx_skp_serial_str2byte(const void * i, int *byte_len){/*{{{*/
    *byte_len = strlen((char *)i);
    ubyte_t * buf = (ubyte_t*)malloc(sizeof(ubyte_t)*(*byte_len));
    strncpy((char*)buf, (char*) i, *byte_len);

    return buf;
}/*}}}*/

void *spx_skp_serial_byte2str(ubyte_t *b, int obj_len){/*{{{*/
    char * buf = (char *)calloc(1, sizeof(char)*(obj_len + 1));
    strncpy(buf, (char*) b, obj_len);

    return buf;
}/*}}}*/

/*
 * public cmp method
 */

int cmp_md(const void *a, const void *b){/*{{{*/
    struct spx_skp_serial_metadata *mda = (struct spx_skp_serial_metadata *) a;
    struct spx_skp_serial_metadata *mdb = (struct spx_skp_serial_metadata *) b;

    if (mda->filename_len  >  mdb->filename_len)
        return 1;
    else if (mda->filename_len < mdb->filename_len)
        return -1;
    else{
        if (strcmp(mda->file, mdb->file) > 0)
            return 1;
        else if (strcmp(mda->file, mdb->file) < 0)
            return -1;
        else{
            if (mda->off > mdb->off)
                return 1; 
            else if (mda->off < mdb->off)
                return -1;
            else{
                if (mda->len > mdb->len)
                    return 1;
                else if (mda->len < mdb->len)
                    return -1;
                else
                    return 0;
            }
        }
    }
}/*}}}*/

int cmp_int(const void *a, const void *b){/*{{{*/
    return *((int *)a) - *((int *)b);
}/*}}}*/

int cmp_long(const void *a, const void *b){/*{{{*/
    return *(long *)a - *(long *)b;
}/*}}}*/

int cmp_float(const void *a, const void *b){/*{{{*/
    return *(float *)a - *(float *)b;
}/*}}}*/

int cmp_double(const void *a, const void *b){/*{{{*/
    return *(double *)a - *(double *)b;
}/*}}}*/

int cmp_str(const void *a, const void *b){/*{{{*/
    return strcmp((char *)a, (char *)b);
}/*}}}*/


