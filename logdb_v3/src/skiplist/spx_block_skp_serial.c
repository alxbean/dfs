/************************************************************************
  > File Name: spx_block_skp_serial.c
  > Author: shuaixiang
  > Mail: shuaixiang@yuewen.com
  > Created Time: Tue 15 Sep 2015 02:37:28 AM UTC
 ************************************************************************/
#include "spx_block_skp_serial.h" 
#include "spx_message.h"
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include "logdb_skp_common.h"

#define SpxSkpSerialMaxLogSize 16777216//the max size of data file  16M
#define SpxSkpSerialBaseSize 20//count(long) + offset(long) + version(int)
#define SpxSkpSerialHeadSize 8388608//8M
#define SpxSkpSerialItemSize 12 //Head block Item size: block_used_len(int) + block_Offset(long)
#define SpxSkpMaxLogCount 1024
#define SpxSkpSerialPathLen 255
#define SpxSkpSerialBlockSize 65535//block 64k

/**********************************************/
/* base_header_format: count|offset|version
 * head_item_format: block_used|block_offset
 * block_index_item_format: key_len|key|value_len|value
 */
/*********************************************/

static int g_file_count = 0; //single thread safe 
static char g_file[SpxSkpSerialFileNameSize];
static FILE *g_fp;

//declare
static int64_t spx_skp_serial_get_file_size(const char * file);
static int spx_skp_serial_expand_file(const char * file , int64_t size);
static int spx_skp_serial_flush_map(struct spx_skp_serial_map_stat *mst);
static ubyte_t *spx_skp_serial_get_header(char *path);
static ubyte_t *spx_skp_serial_get_item(ubyte_t *header, int64_t index);
static struct spx_skp_serial_map_stat *spx_skp_serial_map(const char * file, int64_t size, int64_t offset);
static int spx_skp_serial_un_map(struct spx_skp_serial_map_stat * mst);
static struct spx_skp_serial_metadata *spx_skp_serial_write_data(FILE* fp, char *file, const ubyte_t *data, size_t len);
static int spx_skp_serial_which_file(char *file);
static char *gen_local_time();
static int spx_skp_serial_is_dir_exist(const char *dir_path);
static struct spx_skp_serial_map_stat * spx_skp_serial_map_stat_list_find(const char *path);
static int spx_skp_serial_map_stat_list_insert(const char * path, struct spx_skp_serial_map_stat *mst);
static struct spx_skp_serial_map_stat* spx_skp_serial_init(const char *file);
static void spx_skp_get_idx_path(char *skp_name, char *path, int size);
static SpxSkpO2BDelegate spx_skp_serial_get_o2b(SpxSkpType type);
static SpxSkpB2ODelegate spx_skp_serial_get_b2o(SpxSkpType type);
static int spx_block_skp_node_key_detect(ubyte_t * block, int len, int *off, CmpDelegate cmp_key, void *key, SpxSkpB2ODelegate byte2key, FreeDelegate free_key);
static int spx_block_skp_node_byte_move(ubyte_t * block, int64_t src_off, int64_t dest_off, int len);
static int64_t spx_block_skp_find_empty_block(ubyte_t *header);
static int64_t spx_block_skp_insert_value_block(char *path, ubyte_t *header, int64_t index, int value_len, ubyte_t *value_byte);
static int spx_block_skp_node_find_mid_off(ubyte_t * block, int len);
static bool_t spx_skp_serial_is_index_available(int64_t index);
static struct spx_block_skp_node * spx_block_skp_node_reborn(char *path, int64_t index, ubyte_t *item_ptr, SpxSkpB2ODelegate byte2key);
static int spx_block_skp_serial_value_node_query(char *path, ubyte_t *header, int64_t index, SpxSkpB2ODelegate byte2value, CmpDelegate cmp_value,  struct spx_skp_query_result *result);

struct spx_skp_serial_map_stat_list_node{
    char path[SpxSkpSerialPathLen];
    struct spx_skp_serial_map_stat *mst;
    struct spx_skp_serial_map_stat_list_node * next_mst;
};

struct spx_skp_serial_map_stat_list{
    struct spx_skp_serial_map_stat_list_node *head;
    struct spx_skp_serial_map_stat_list_node *tail;
} spx_skp_serial_map_stat_list_head;

/*
 * private method
 */
static int spx_skp_serial_map_stat_list_insert(const char * path, struct spx_skp_serial_map_stat *mst){/*{{{*/
    if (NULL == path || NULL == mst){
        printf("path or spx_skp_serial_map_stat is NULL\n");
        return -1;
    }

    struct spx_skp_serial_map_stat_list_node *new_mst = (struct spx_skp_serial_map_stat_list_node *) malloc(sizeof(struct spx_skp_serial_map_stat_list_node));
    new_mst->mst = mst;
    strncpy(new_mst->path, path, strlen(path)); 
    *(new_mst->path + strlen(path)) = '\0';
    new_mst->next_mst = NULL;

    if (NULL == spx_skp_serial_map_stat_list_head.head){
        spx_skp_serial_map_stat_list_head.head = new_mst;
        spx_skp_serial_map_stat_list_head.tail = new_mst;
    } else if (spx_skp_serial_map_stat_list_head.tail != NULL){
        spx_skp_serial_map_stat_list_head.tail->next_mst = new_mst;
        spx_skp_serial_map_stat_list_head.tail = new_mst;
    } else {
        printf("mst list tail is NULL\n");
        return -1;
    }

    return 0;
}/*}}}*/

static struct spx_skp_serial_map_stat * spx_skp_serial_map_stat_list_find(const char *path){/*{{{*/
    struct spx_skp_serial_map_stat_list_node *tmp_mst = spx_skp_serial_map_stat_list_head.head;
    while (NULL != tmp_mst){
        if (!strcmp(tmp_mst->path, path)){
            return tmp_mst->mst;
        } 

        tmp_mst = tmp_mst->next_mst;
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

//get header 
static ubyte_t* spx_skp_serial_get_header(char *path){/*{{{*/
    if (NULL == path){
        printf("name is NULL in spx_skp_serial_get_header\n");
        return NULL;
    }

    struct spx_skp_serial_map_stat *mst = spx_skp_serial_init(path);
    return mst->mapped;
}/*}}}*/

//get index item
static ubyte_t *spx_skp_serial_get_item(ubyte_t *header, int64_t index){/*{{{*/
    if (NULL == header){
        printf("header is NULL in spx_skp_serial_get_item\n");
        return NULL;
    }

    if (false == spx_skp_serial_is_index_available(index)){
        printf("index is not available in spx_skp_serial_get_item\n");
        return NULL;
    }

    ubyte_t *start_ptr = header + SpxSkpSerialBaseSize;
    ubyte_t *item_ptr = start_ptr + index * SpxSkpSerialItemSize;
    return item_ptr;
}/*}}}*/

/*
* mapped init
*/
//flush map
static int spx_skp_serial_flush_map(struct spx_skp_serial_map_stat *mst){/*{{{*/

    if ((msync((void *) (mst->mapped - mst->distance), (mst->size + mst->distance), MS_SYNC)) == -1){
        return -1;
    }

    return 0;
}/*}}}*/

//unmap
static int spx_skp_serial_un_map(struct spx_skp_serial_map_stat * mst){/*{{{*/
    if ((munmap((void *)(mst->mapped - mst->distance), (mst->size + mst->distance))) == -1){
        printf("munmap file failed\n");
        free(mst);
        return -1;
    }

    free(mst);
    return 0;
}/*}}}*/

//map 
static struct spx_skp_serial_map_stat *spx_skp_serial_map(const char * file, int64_t size, int64_t offset){/*{{{*/
    int fd;
    int64_t file_size = spx_skp_serial_get_file_size(file);
    int64_t page_size = sysconf(_SC_PAGE_SIZE);
    int64_t expand_size = size;
    int i = 1;
    ubyte_t *mapped;
    struct spx_skp_serial_map_stat *mst = (struct spx_skp_serial_map_stat*) malloc(sizeof(struct spx_skp_serial_map_stat));

    while ((file_size - offset) <= size){
        spx_skp_serial_expand_file(file, expand_size);
        file_size = spx_skp_serial_get_file_size(file);
        expand_size = page_size * i++;
    }

    int64_t align_offset = (offset/page_size)*page_size;
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

    mst->mapped = mapped + distance;
    mst->size = size;
    mst->distance = distance;

    return mst;
}/*}}}*/

//index file init
static struct spx_skp_serial_map_stat *spx_skp_serial_init(const char *file){/*{{{*/
    if (NULL == file){
        printf("file to init is NULL\n");
        return NULL;
    }

    struct spx_skp_serial_map_stat *mst = spx_skp_serial_map_stat_list_find(file);

    if (NULL != mst)
        return mst;

    if (access(file, 0)){
        if(spx_skp_serial_expand_file(file, SpxSkpSerialHeadSize) == -1)
            return NULL;

        struct spx_skp_serial_map_stat* mst = spx_skp_serial_map(file, SpxSkpSerialBaseSize, 0); 
        ubyte_t* mp = mst->mapped; 
        spx_msg_l2b(mp, 0);//count
        spx_msg_l2b(mp + 8, SpxSkpSerialHeadSize);//offset
        spx_msg_i2b(mp + 16, 0);//version
        if (0 != spx_skp_serial_un_map(mst)){
            printf("spx_skp_serial_un_map failed\n");
        }
    }

    mst = spx_skp_serial_map(file, SpxSkpSerialHeadSize, 0);
    spx_skp_serial_map_stat_list_insert(file, mst);

    return mst;
}/*}}}*/

/*
 *Log persistence
 */

struct spx_skp_serial_metadata_list * spx_skp_serial_metadata_list_new(){/*{{{*/
    struct spx_skp_serial_metadata_list *md_lst = (struct spx_skp_serial_metadata_list *) malloc(sizeof(*md_lst));    
    if (NULL == md_lst){
        printf("malloc spx_skp_serial_metadata_list failed\n");
        return NULL;
    }

    md_lst->head = NULL;
    md_lst->tail = NULL;

    return md_lst;
}/*}}}*/

int spx_skp_serial_metadata_list_insert(struct spx_skp_serial_metadata_list * md_lst, struct spx_skp_serial_metadata *md){/*{{{*/
    if (NULL == md_lst){
        printf("md_lst is NULL\n");
        return -1;
    }

    if (NULL == md){
        printf("spx_skp_serial_metadata_list_insert md is NULL\n");
        return -1;
    }

    struct spx_skp_serial_metadata_list_node * new_md_node = (struct spx_skp_serial_metadata_list_node *) malloc(sizeof(*new_md_node));
    if (NULL == new_md_node){
        printf("malloc spx_skp_serial_metadata_list_node failed\n");
        return -1;
    }

    new_md_node->md = md;
    new_md_node->next_md = NULL;

    if (NULL == md_lst->tail){
        md_lst->head = new_md_node;
        md_lst->tail = new_md_node;
    } else {
        md_lst->tail->next_md = new_md_node;
        md_lst->tail = new_md_node;
    }

    return 0;
}/*}}}*/

int spx_skp_serial_metadata_list_free(struct spx_skp_serial_metadata_list * md_lst, bool_t is_free_md){/*{{{*/
    if (NULL == md_lst){
        printf("md_lst is NULL\n");
        return -1;
    }

    struct spx_skp_serial_metadata_list_node * tmp_md = md_lst->head;

    while(tmp_md){
        struct spx_skp_serial_metadata_list_node * spx_skp_serial_free_md = tmp_md;
        if (true == is_free_md)
            spx_skp_serial_md_free(tmp_md->md);
        tmp_md = tmp_md->next_md;
        free(spx_skp_serial_free_md);
    }

    free(md_lst);

    return 0;
}/*}}}*/

static struct spx_skp_serial_metadata *spx_skp_serial_write_data(FILE* fp, char* file, const ubyte_t *data, size_t len){/*{{{*/
    //printf("writing data...\n");
    fwrite(data, sizeof(char), len, fp);
    //fflush(fp);
    //printf("writing data done\n");

    struct spx_skp_serial_metadata *md = (struct spx_skp_serial_metadata*) malloc(sizeof(struct spx_skp_serial_metadata));
    int file_len = strlen(file);
    strncpy(md->file, file, file_len);
    *(md->file + file_len) = '\0';
    md->len = len;
    md->off = ftell(fp) - len;
    
    return md;
}/*}}}*/

static int spx_skp_serial_which_file(char *file){/*{{{*/
    char path[SpxSkpSerialFileNameSize];
    char path1[SpxSkpSerialFileNameSize];
    char path2[SpxSkpSerialFileNameSize];
    char time_path[SpxSkpSerialFileNameSize];
    getcwd(path, sizeof(path));
    if (-1 == spx_skp_serial_build_time_dir(time_path))
        return -1;
    snprintf(path1, sizeof(path1), "%s/skiplist/data/%s", path, time_path);
    snprintf(path2, sizeof(path2), "%s/%04d.log", path1, g_file_count);

    while( (spx_skp_serial_get_file_size(path2) > SpxSkpSerialMaxLogSize && g_file_count < SpxSkpMaxLogCount))
        snprintf(path2, sizeof(path2), "%s/%04d.log", path1, ++g_file_count);

    if (g_file_count >= SpxSkpMaxLogCount){
        printf("file_count is full\n");
        return -1;
    }

    snprintf(file, SpxSkpSerialFileNameSize, "%s/%04d.log", time_path, g_file_count);
    return 0;
}/*}}}*/

//ubyte_t * spx_skp_serial_data_writer2byte(const ubyte_t *data, size_t len){/*{{{*/
//    char file[SpxSkpSerialFileNameSize] = {0};// yyyy/mm/dd\n
//    if (spx_skp_serial_which_file(file) != 0){
//        printf("locate file to write failed\n");
//        return NULL;
//    }
//
//    if (strcmp(file, g_file)){
//        strncpy(g_file, file, strlen(file));
//        char path[SpxSkpSerialFileNameSize];
//        char path1[SpxSkpSerialFileNameSize]
//        getcwd(path, sizeof(path));
//        snprintf(path1, sizeof(path1), "%s/skiplist/data/%s", path, g_file);
//        if (g_fp != NULL)
//            fclose(g_fp);
//        g_fp = fopen(path1, "a+");
//        if (NULL == g_fp){
//            printf("opening file:%s failed\n", path1);
//            perror("fp is NULL");
//            return NULL
//        }
//    }
//
//    struct spx_skp_serial_metadata *md = spx_skp_serial_write_data(fp, file, data, len);
//    if(NULL == md){
//        printf("write data error\n");
//        return NULL;
//    }
//    ubyte_t *buf = spx_skp_serial_m2b(md, NULL);
//
//    spx_skp_serial_md_free(md);
//    return buf;
//}/*}}}*/

struct spx_skp_serial_metadata* spx_skp_serial_data_writer2md(const ubyte_t *data, size_t len){/*{{{*/
    char file[SpxSkpSerialFileNameSize] = {0};// yyyy/mm/dd\n
    if (spx_skp_serial_which_file(file) != 0){
        printf("locate file to write failed\n");
        return NULL;
    }

    if (strcmp(file, g_file)){
        strncpy(g_file, file, strlen(file));
        char path[SpxSkpSerialFileNameSize];
        char path1[SpxSkpSerialFileNameSize];
        getcwd(path, sizeof(path));
        snprintf(path1, sizeof(path1), "%s/skiplist/data/%s", path, g_file);
        if (g_fp != NULL)
            fclose(g_fp);
        g_fp = fopen(path1, "a+");
        if (NULL == g_fp){
            printf("opening file:%s failed\n", path1);
            perror("fp is NULL");
            return NULL;
        }
    }

    struct spx_skp_serial_metadata *md = spx_skp_serial_write_data(g_fp, g_file, data, len);
    if(NULL == md){
        printf("write data error\n");
        return NULL;
    }

    return md;
}/*}}}*/

struct spx_skp_serial_metadata *spx_skp_serial_md_copy(struct spx_skp_serial_metadata * src_md){/*{{{*/
    if (NULL == src_md){
        printf("copy md error\n");
        return NULL;
    }

    struct spx_skp_serial_metadata *new_md= (struct spx_skp_serial_metadata*) malloc(sizeof(struct spx_skp_serial_metadata));
    if (NULL == new_md){
        printf("malloc new_md failed\n");
        return NULL;
    }

    int file_len = strlen(src_md->file);
    strncpy(new_md->file, src_md->file, file_len);
    *(new_md->file + file_len) = '\0';
    new_md->len = src_md->len;
    new_md->off = src_md->off;
    return new_md;
}/*}}}*/

void spx_skp_serial_md_free(struct spx_skp_serial_metadata *md){/*{{{*/
    if (NULL == md){
        printf("md is NULL\n");
        return;
    }

    free(md);
}/*}}}*/

//read data
ubyte_t * spx_skp_serial_data_reader(struct spx_skp_serial_metadata *md){/*{{{*/
    char filename_len = strlen(md->file);
    char *tmp_file = md->file;
    int64_t off = md->off;
    int64_t len = md->len;

    char dir_path[SpxSkpSerialFileNameSize];
    char file[SpxSkpSerialFileNameSize];
    getcwd(dir_path, sizeof(dir_path));
    snprintf(file, sizeof(file), "%s/skiplist/data/%s", dir_path, tmp_file);

    FILE *fp = fopen(file, "r+");
    if (NULL == fp){
        printf("opening file :%s\n", file);
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

//get o2b, b2o with type

static SpxSkpO2BDelegate spx_skp_serial_get_o2b(SpxSkpType type){/*{{{*/
    SpxSkpO2BDelegate object2byte = NULL;

    switch(type){
        case SKP_MD:
            object2byte = spx_block_skp_common_md2byte;
            break;
        case SKP_STR:
            object2byte = spx_block_skp_common_str2byte;
            break;
        case SKP_INT:
            object2byte = spx_block_skp_common_int2byte;
            break;
        case SKP_LONG:
            object2byte = spx_block_skp_common_long2byte;
            break;
        default:
                printf("not support type\n");
    }

    return object2byte;
}/*}}}*/

static SpxSkpB2ODelegate spx_skp_serial_get_b2o(SpxSkpType type){/*{{{*/
    SpxSkpB2ODelegate byte2object = NULL;

    switch(type){
        case SKP_MD:
            byte2object = spx_block_skp_common_byte2md;
            break;
        case SKP_STR:
            byte2object = spx_block_skp_common_byte2str;
            break;
        case SKP_INT:
            byte2object = spx_block_skp_common_byte2int;
            break;
        case SKP_LONG:
            byte2object = spx_block_skp_common_byte2long;
            break;
        default:
                printf("not support type\n");
    }

    return byte2object;
}/*}}}*/

//b2m, m2b
ubyte_t * spx_skp_serial_m2b(struct spx_skp_serial_metadata *md, int *byteLen){/*{{{*/
    int filename_len = strlen(md->file);
    int size  = 1 + filename_len + 8 + 8; 
    ubyte_t * b = (ubyte_t *)calloc(1, sizeof(ubyte_t) * size);    
    *b = (filename_len & 0xff);
    memcpy(b + 1, md->file, filename_len);
    spx_msg_l2b((b + 1 + filename_len), md->off);
    spx_msg_l2b((b + 1 + filename_len + 8), md->len);

    if (NULL != byteLen)
        *byteLen = size;
    return b;
}/*}}}*/

struct spx_skp_serial_metadata *spx_skp_serial_b2m(ubyte_t *b){/*{{{*/
    struct spx_skp_serial_metadata *md= (struct spx_skp_serial_metadata*) calloc(1, sizeof(struct spx_skp_serial_metadata));
    if(NULL == md){
        printf("malloc struct spx_skp_serial_metadata failed");
        return NULL;
    }

    char filename_len = *b; 
    int64_t off = spx_msg_b2l(b + 1 + filename_len);
    int64_t len = spx_msg_b2l(b + 1 + filename_len + 8);

    memcpy(md->file, (b + 1), filename_len);
    *(md->file + filename_len) = '\0';
    md->len = len;
    md->off = off;
    return md;
}/*}}}*/


/*
 * block skiplist serial 
 */

//serial_context
struct spx_block_skp_node_serial_context *spx_block_serial_context_new(){/*{{{*/
    struct spx_block_skp_node_serial_context *serial_ctx = (struct spx_block_skp_node_serial_context*) malloc(sizeof(*serial_ctx));
    if (NULL == serial_ctx){
        printf("malloc serial_ctx failed\n");
        return NULL;
    }

    serial_ctx->old_left_key = NULL;
    serial_ctx->old_right_key = NULL;
    serial_ctx->new_left_key = NULL;
    serial_ctx->new_right_key = NULL;

    return serial_ctx;
}/*}}}*/

int spx_block_serial_context_free(struct spx_block_skp_node_serial_context **serial_ctx){/*{{{*/
    if (NULL == serial_ctx || NULL == *serial_ctx){
        printf("free serial_ctx failed\n");
        return -1;
    }

    free(*serial_ctx);
    *serial_ctx = NULL;

    return 0;
}/*}}}*/


static int spx_block_skp_node_key_detect(ubyte_t * block, int len, int *off, CmpDelegate cmp_key, void *key, SpxSkpB2ODelegate byte2key, FreeDelegate free_key){/*{{{*/
    if (NULL == block){
        printf("spx_block_skp_node_key_detect block is NULL\n");
        return -1;
    }

    if (NULL == key){
        printf("spx_block_skp_node_key_detect key is NULL\n");
        return -1;
    }

    while(*off < len){
        ubyte_t *pos = block + *off;
        int key_len = spx_msg_b2i(pos);
        void *tmp_key = byte2key(pos + 4, key_len); 
        if (NULL == tmp_key){
            printf("tmp_key is NULL\n");
            return -1;
        }

        int result = cmp_key(key, tmp_key);
        free_key(tmp_key);

        if (0 == result){//find key already exist
            return 0;
        } else if (result < 0){//find the off to insert, and the following need to move
            return 1;
        }

        int value_len = spx_msg_b2i(pos + 4 + key_len + 8);
        int tmp_len = 4 + key_len + 8 + 4 + value_len;
        *off += tmp_len;
    }

    return 1;
}/*}}}*/

static int spx_block_skp_node_less_off_detect(ubyte_t * block, int len, int less_off){/*{{{*/
    if (NULL == block){
        printf("spx_block_skp_node_less_off_detect block is NULL\n");
        return -1;
    }

    if (len < less_off){
        printf("off is not reach yet\n");
        return -1;
    }

    int off = 0;
    int tmp_len = 0;

    while ((off + tmp_len) < less_off){
        off += tmp_len;
        ubyte_t *pos = block + off;
        int key_len = spx_msg_b2i(pos);
        int value_len = spx_msg_b2i(pos + 4 + key_len + 8);
        tmp_len = 4 + key_len + 8 + 4 + value_len;
    }

    return off;
}/*}}}*/

//Attention: len must be the distance between src_off and index_len
static int spx_block_skp_node_byte_move(ubyte_t * block, int64_t src_off, int64_t dest_off, int len){/*{{{*/
    if (0 == len){
        return 0;
    }
    if (len < 0){
        printf("move len in spx_block_skp_node_byte_move less than 0\n");
        return -1;
    }
    ubyte_t * tmp_mem = (ubyte_t*)calloc(sizeof(ubyte_t), len);
    if (NULL == tmp_mem){
        printf("calloc in spx_block_skp_node_byte_move failed\n");
        return -1;
    }

    memcpy(tmp_mem, block + src_off, len);
    memcpy(block + dest_off, tmp_mem, len);
    free(tmp_mem);

    return 0;
}/*}}}*/

static bool_t spx_skp_serial_is_index_available(int64_t index){/*{{{*/
    int64_t max_count = (SpxSkpSerialHeadSize/SpxSkpSerialItemSize);
    if (index < 0 || index > max_count){
        printf("index is invalid\n");
        return false;
    } 

    return true;
}/*}}}*/

static int64_t spx_block_skp_find_empty_block(ubyte_t* header){/*{{{*/
    if (NULL == header){
        printf("header is NULL\n");
        return -1;
    }

    int64_t count = spx_msg_b2l(header); //count of blocks
    int64_t offset = spx_msg_b2l(header + 8);// new block offset
    int version = spx_msg_b2i(header + 16);//TODO: vserion of skiplist, int will out of the range
    int64_t index = count;
    if (false == spx_skp_serial_is_index_available(index)){
        printf("index is invaild, maybe headitem is full\n");
        return -1;
    }

    spx_msg_i2b(header + 16, ++version);
    ubyte_t* item_ptr = spx_skp_serial_get_item(header, index);
    spx_msg_i2b(item_ptr, 0);//set block_used_len
    spx_msg_l2b(item_ptr + 4, offset); //set block offset

    spx_msg_l2b(header, ++count);//increase block count
    spx_msg_l2b(header + 8, offset + SpxSkpSerialBlockSize);//init new block offset 

    return index;
}/*}}}*/

static int64_t spx_block_skp_insert_value_block(char *path, ubyte_t *header, int64_t index, int value_len, ubyte_t *value_byte){/*{{{*/
    if (NULL == header){
        printf("header is NULL\n");
        return false;
    }

    if ((SpxSkpSerialBlockSize - 4 - 8) < (4 + value_len)){
        printf("value is too long to save in a block, please adjust the Block size to bigger than %d\n", value_len);
        return -1;
    }

    if (-1 == index){
        index = spx_block_skp_find_empty_block(header, VALUE);
        if (false == spx_skp_serial_is_index_available(index)){
            return -1;
        }

        ubyte_t *index_item_ptr = spx_skp_serial_get_item(header, index); 
        spx_skp_serial_index_type index_type = *index_item_ptr;
        if (INDEX == index_type){
            printf("find_value_block error\n");
            return -1;
        }

        int64_t index_offset = spx_msg_b2l(index_item_ptr + 5);
        struct spx_skp_serial_map_stat *block_mst = spx_skp_serial_map(path, SpxSkpSerialBlockSize, index_offset);  
        ubyte_t * insert_ptr = block_mst->mapped + 4 + 8;//value_count|value_index
        spx_msg_i2b(insert_ptr, value_len);
        memcpy(insert_ptr + 4, value_byte, value_len); 
        spx_msg_i2b(block_mst->mapped, 1);
        spx_msg_l2b(block_mst->mapped + 4, -1);
        spx_skp_serial_un_map(block_mst);
        spx_msg_i2b(index_item_ptr + 1, (4 + 8) + (4 + value_len));
    } else {
        ubyte_t *index_item_ptr = spx_skp_serial_get_item(header, index); 
        spx_skp_serial_index_type index_type = *index_item_ptr;
        if (INDEX == index_type){
            printf("find_value_block error\n");
            return -1;
        }

        int index_len = spx_msg_b2i(index_item_ptr + 1);
        int64_t index_offset = spx_msg_b2l(index_item_ptr + 5);
        struct spx_skp_serial_map_stat *block_mst = spx_skp_serial_map(path, SpxSkpSerialBlockSize, index_offset);  
        int value_count = spx_msg_b2i(block_mst->mapped);
        if (SpxSkpSerialBlockSize - index_len >= (4 + value_len)){
            ubyte_t *insert_ptr = block_mst->mapped + index_len;
            spx_msg_i2b(insert_ptr, value_len);
            memcpy(insert_ptr + 4, value_byte, value_len); 
            spx_msg_i2b(block_mst->mapped, ++value_count);
            spx_skp_serial_un_map(block_mst);

            spx_msg_i2b(index_item_ptr + 1, index_len + (4 + value_len));
        } else {
            int64_t next_v_idx = spx_msg_b2l(block_mst->mapped + 4);
            spx_skp_serial_un_map(block_mst);
            index = spx_block_skp_insert_value_block(path, header, next_v_idx, value_len, value_byte); 
            if (-1 == next_v_idx && (true == spx_skp_serial_is_index_available(index))){
                struct spx_skp_serial_map_stat *block_mst = spx_skp_serial_map(path, SpxSkpSerialBlockSize, index_offset);  
                spx_msg_l2b(block_mst->mapped + 4, index);
                spx_skp_serial_un_map(block_mst);
            }
        }
    }

    return index;
}/*}}}*/

static int64_t spx_block_node_split(const char *path, struct spx_skp_serial_map_stat * block_mst, int off, ubyte_t *split_index_item_ptr, ubyte_t *header){/*{{{*/
    int64_t new_index = spx_block_skp_find_empty_block(header, INDEX);
    if (-1 == new_index){
        printf("find empty block failed\n");
        return -1;
    }
    int split_index_len = spx_msg_b2i(split_index_item_ptr + 1);
    int split_distance = split_index_len - off;

    if (split_distance <= 0){
        printf("split_distance is less than 0\n");
        return -1;
    }

    if (new_index < 0){
        printf("failed to get a new_index\n");
        return -1;
    } else {
        ubyte_t *new_index_item_ptr = spx_skp_serial_get_item(header, new_index); 
        int64_t new_index_offset = spx_msg_b2l(new_index_item_ptr + 5);
        struct spx_skp_serial_map_stat *new_block_mst = spx_skp_serial_map(path, SpxSkpSerialBlockSize, new_index_offset);  
        memcpy(new_block_mst->mapped, block_mst->mapped + off, split_distance);
        spx_msg_i2b(new_index_item_ptr + 1, split_distance);
        spx_skp_serial_un_map(new_block_mst);
        spx_msg_i2b(split_index_item_ptr + 1, off); 
    }

    return new_index;
}/*}}}*/

static struct spx_block_skp_node * spx_block_skp_node_reborn(char *path, int64_t index, ubyte_t *item_ptr, SpxSkpB2ODelegate byte2key){/*{{{*/
    int index_len = spx_msg_b2i(item_ptr + 1);
    int64_t index_offset = spx_msg_b2l(item_ptr + 5);
    int level = spx_skp_level_rand();

    struct spx_skp_serial_map_stat *block_mst = spx_skp_serial_map(path, SpxSkpSerialBlockSize, index_offset);  
    int left_key_off = 0;
    int left_key_len = spx_msg_b2i(block_mst->mapped + left_key_off);
    void *left_key = byte2key(block_mst->mapped + left_key_off + 4, left_key_len);

    int right_key_off = spx_block_skp_node_less_off_detect(block_mst->mapped, index_len, index_len);
    int right_key_len = spx_msg_b2i(block_mst->mapped + right_key_off);
    void *right_key = byte2key(block_mst->mapped + right_key_off + 4, right_key_len);

    spx_skp_serial_un_map(block_mst);
    return spx_block_skp_node_new(level, left_key, right_key, index);
}/*}}}*/
int spx_block_skp_unserial(struct spx_block_skp *block_skp){/*{{{*/
    if (NULL == block_skp){
        printf("block_skp is NULL to unserial\n");
        return -1;
    } 

    char path[SpxSkpSerialPathLen];
    spx_skp_get_idx_path(block_skp->name, path, sizeof(path));
    ubyte_t *header = spx_skp_serial_get_header(path);
    if (NULL == header){
        printf("header is NULL\n");
        return -1;
    }

    SpxSkpB2ODelegate byte2key = spx_skp_serial_get_b2o(block_skp->key_type);
    if (NULL == byte2key){
        printf("type %d is not supported\n", block_skp->key_type);
        return -1;
    }

    int64_t count = spx_msg_b2l(header); //counts of HEAD
    int64_t i;
    for(i = 0; i < count; i++){
        ubyte_t *item_ptr = spx_skp_serial_get_item(header, i);
        spx_skp_serial_index_type index_type = *item_ptr;
        if (index_type != INDEX)
            continue;

        struct spx_block_skp_node * node = spx_block_skp_node_reborn(path, i, item_ptr, byte2key);
        if ( -1 == spx_block_skp_node_insert(block_skp, node)){
            printf("block_skp insert node failed\n");
        }
    }

    return 0;
}/*}}}*/


//re_construct
static int spx_block_skp_serial_append(ubyte_t* block_tail, int key_len, ubyte_t* key_byte, int value_len, ubyte_t* value_byte){/*{{{*/
    if (NULL == block_item){
        printf("block_item is NULL in spx_block_skp_serial_append\n");
        return -1;
    }

    if (NULL == block_tail){
        printf("block_tail is NULL in spx_block_serial_append\n");
        return -1;
    }

    if (NULL == key_byte){
        printf("key_byte is NULL in spx_block_skp_serial_append\n");
        return -1;
    }

    if (NULL == value_byte){
        printf("value_byte is NULL in spx_block_skp_serial_append\n");
        return -1;
    }

    spx_msg_i2b(block_tail, key_len);
    memcpy(block_tail + 4, key_byte, key_len);
    spx_msg_i2b(block_tail + 4 + key_len, value_len);
    memcpy(block_tail + 4 + key_len + 4, value_byte, value_len);


    int block_len = spx_msg_b2i(block_item);
    int item_len = 4 + key_len + 4 + value_len;
    spx_msg_i2b(block_item, block_len + item_len); 

    return 0;
}/*}}}*/

static int64_t spx_block_skp_serial_split(){

}

int64_t spx_block_skp_serial(struct spx_block_skp *block_skp, void *key, void * value, int64_t index, struct spx_block_skp_node_serial_context *serial_ctx){/*{{{*/
    if (NULL == block_skp){
        printf("block_skp is NULL in spx_block_skp_node_serial\n");
        return -1;
    }

    if (index < -1){
        printf("spx_block_skp_node_serial index is out of range\n");
        return -1;
    }
    
    if (NULL == serial_ctx){
        printf("serial_ctx in spx_block_skp_node_serial is NULL\n");
        return -1;
    }

    char path[SpxSkpSerialPathLen];
    spx_skp_get_idx_path(block_skp->name, path, sizeof(path));
    ubyte_t* header = spx_skp_serial_get_header(path);
    int64_t ret = -2;
    if (NULL == header){
        printf("header is NULL\n");
        return -1;
    }

    int key_len = 0;
    int value_len = 0;

    SpxSkpO2BDelegate key2byte = spx_skp_serial_get_o2b(block_skp->key_type);
    SpxSkpO2BDelegate value2byte = spx_skp_serial_get_o2b(block_skp->value_type);
    if (NULL == key2byte){
        printf("type %d is not supported\n", block_skp->key_type);
        return -1;
    }

    if (NULL == value2byte){
        printf("type %d is not supported\n", block_skp->value_type);
        return -1;
    }

    SpxSkpB2ODelegate byte2key = spx_skp_serial_get_b2o(block_skp->key_type);
    if (NULL == byte2key){
        printf("type %d is not supported\n", block_skp->key_type);
        return -1;
    }

    ubyte_t* key_byte = key2byte(key, &key_len);
    ubyte_t* value_byte = value2byte(value, &value_len);
    int item_len = 4 + key_len + 4 + value_len;
    if (item_len > (SpxSkpSerialBlockSize/2)){
        printf("the length of key and value is out of range\n");
        return -1;
    }

    int64_t count = spx_msg_b2l(header); //counts of blocks 
    int64_t offset = spx_msg_b2l(header + 8);// offset of last block 
    int version = spx_msg_b2i(header + 16);//vserion of skiplist

    if (-1 == index){
        //need a new block
        index = spx_block_skp_find_empty_block(header);
        if (-1 == index){
            free(key_byte);
            free(value_byte);
            return -1;
        }

        ret = index;
    }

    ubyte_t* block_item = spx_skp_serial_get_item(header, index);
    int block_len = spx_msg_b2i(block_item);
    int64_t block_offset = spx_msg_b2l(block_item + 4);
    int free_block = SpxSkpSerialBlockSize - block_len;

    struct spx_skp_serial_map_stat* block_mst = spx_skp_serial_map(path, SpxSkpSerialBlockSize, block_offset);  

    if (free_block >= item_len){
        spx_block_skp_serial_append(block_mst->mapped + block_len, key_len, key_byte, value_len, value_byte);
    } else {

    }

    free(key_byte);
    free(value_byte);
    spx_skp_serial_un_map(block_mst);

    return ret;
r1:
    free(key_byte);
    free(value_byte);
    spx_skp_serial_un_map(block_mst);
    return -1;
}/*}}}*/



/*
 *spx_block_skp_node_query
 */
static int spx_block_skp_serial_value_node_query(char *path, ubyte_t *header, int64_t index, SpxSkpB2ODelegate byte2value, CmpDelegate cmp_value,  struct spx_skp_query_result *result){/*{{{*/
    ubyte_t *item_ptr = spx_skp_serial_get_item(header, index);
    spx_skp_serial_index_type index_type = *item_ptr;
    if (index_type != VALUE){
        printf("index:%ld is not INDEX\n", index);
        return -1;
    }
    int index_len = spx_msg_b2i(item_ptr + 1);
    int64_t index_offset = spx_msg_b2l(item_ptr + 5);

    struct spx_skp_serial_map_stat *block_mst = spx_skp_serial_map(path, SpxSkpSerialBlockSize, index_offset);  
    int count = spx_msg_b2i(block_mst->mapped);
    int64_t next_idx = spx_msg_b2l(block_mst->mapped + 4);
    int i;
    int pos = 4 + 8;

    for (i = 0; i < count; i++){
        ubyte_t *value_ptr = block_mst->mapped + pos;
        int value_len = spx_msg_b2i(value_ptr);
        void *value = byte2value(value_ptr + 4, value_len);
        //if (0 == spx_skp_query_result_is_exist(result, value, cmp_value))
            spx_skp_query_result_insert(result, value);
        pos += (4 + value_len);
    }
    spx_skp_serial_un_map(block_mst);

    if (true == spx_skp_serial_is_index_available(next_idx)){
        return spx_block_skp_serial_value_node_query(path, header, next_idx, byte2value, cmp_value, result); 
    }

    return 0;
}/*}}}*/

int spx_block_skp_serial_node_query(struct spx_block_skp *block_skp, void *key, int64_t index, struct spx_skp_query_result *result){/*{{{*/
    if (NULL == block_skp){
        printf("block_skp is NULL in spx_block_skp_serial_node_query\n");
        return -1;
    }

    if (NULL == key){
        printf("key is NULL in spx_block_skp_serial_node_query\n");
        return -1;
    }

    if (NULL == result){
        printf("result is NULL in spx_block_skp_serial_node_query\n");
        return -1;
    }

    if (false == spx_skp_serial_is_index_available(index)){
        printf("spx_block_skp_node_serial index is out of range\n");
        return -1;
    }

    SpxSkpB2ODelegate byte2key = spx_skp_serial_get_b2o(block_skp->key_type);
    if (NULL == byte2key){
        printf("type %d is not supported\n", block_skp->key_type);
        return -1;
    }

    SpxSkpB2ODelegate byte2value = spx_skp_serial_get_b2o(block_skp->value_type);
    if (NULL == byte2value){
        printf("type %d is not supported\n", block_skp->value_type);
        return -1;
    }

    char path[SpxSkpSerialPathLen];
    spx_skp_get_idx_path(block_skp->name, path, sizeof(path));
    ubyte_t *header = spx_skp_serial_get_header(path);
    if (NULL == header){
        printf("header is NULL\n");
        return -1;
    }

    ubyte_t *item_ptr = spx_skp_serial_get_item(header, index);
    spx_skp_serial_index_type index_type = *item_ptr;
    if (index_type != INDEX){
        printf("the index:%ld item is not a INDEX type\n", index);
        return -1;
    }

    int index_len = spx_msg_b2i(item_ptr + 1);
    int64_t index_offset = spx_msg_b2l(item_ptr + 5);
    struct spx_skp_serial_map_stat *block_mst = spx_skp_serial_map(path, SpxSkpSerialBlockSize, index_offset);  
    int off = 0;
    int flag = spx_block_skp_node_key_detect(block_mst->mapped, index_len, &off, block_skp->cmp_key, key, byte2key, block_skp->free_key);
    if (flag ==0){
        ubyte_t * key_ptr = block_mst->mapped + off;
        int key_len = spx_msg_b2i(key_ptr);
        int64_t value_index = spx_msg_b2l(key_ptr + 4 + key_len);
        int value_len = spx_msg_b2i(key_ptr + 4 + key_len + 8);
        void *value = byte2value(key_ptr + 4 + key_len + 8 + 4, value_len); 
        //if (0 == spx_skp_query_result_is_exist(result, value, block_skp->cmp_value))
            spx_skp_query_result_insert(result, value);
        if (value_index != -1){
            spx_block_skp_serial_value_node_query(path, header, value_index, byte2value, block_skp->cmp_value, result);
        }
    }

    spx_skp_serial_un_map(block_mst);
    return 0;
}/*}}}*/

