/************************************************************************
    > File Name: serial.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Tue 15 Sep 2015 02:37:28 AM UTC
 ************************************************************************/
#include "serial.h" 

//declare
static long get_file_size(const char * file);
static int expand_file(const char * file , int size);
static int sync_map(char * mapped, const char * file);
static int un_map(char * mapped, int size);
static char *map_file(const char * file, long size, long offset);
static void i2b(char * a, int b);
static int b2i(char *b);
static int write_value(FILE *fp, char *buf);

//define
/*Mapped Init*/
//get FileSize
static long get_file_size(const char * file){
    struct stat sb;
    if ( stat(file, &sb) == -1 ){
        perror("stat");
    }

    return sb.st_size;
}

//increase FileSize
static int expand_file(const char * file , int size){
    FILE *fp = fopen(file, "a+"); 
    if(fp == NULL)
        return -1;
    fseek(fp, size-1, SEEK_END);    

    for(int i = 0; i < size; i++)
        fwrite("", 1, 1, fp);

    fclose(fp);

    return 0;
}

//flush map
static int sync_map(char * mapped, const char * file){
    
    if ((msync((void *) mapped, get_file_size(file), MS_SYNC)) == -1){
        return -1;
    }

    return 0;
}

//unMap
static int un_map(char * mapped, int size){
    if ((munmap((void *)mapped, size)) == -1){
        return -1;
    }

    return 0;
}

//map 
static char *map_file(const char * file, long size, long offset){
    int fd;
    char * mapped;
    long fileSize = get_file_size(file);
    
    if( (fileSize - offset) < size)
        expand_file(file, size*2);

    int page_size = sysconf(_SC_PAGE_SIZE);
    int align_offset = (offset/page_size)*page_size;
    int distance = offset - align_offset;

    if((fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) < 0){
        perror("open");
    }

    if ((mapped = (char *)mmap(NULL, size, PROT_READ | 
                               PROT_WRITE, MAP_SHARED, fd, align_offset)) == (void *) -1){
                                   perror("mmap");
    }

    close(fd);
    return mapped + distance;

}

/*map OP*/
//int 2 byte
static void i2b(char * a, int b){
    *a++ = (b>>24) & 0xFF;
    *a++ = (b>>16) & 0xFF;
    *a++ = (b>>8)  & 0xFF;
    *a++ = b & 0xFF;
}

//byte 2 int
static int b2i(char *b){
    return ((((unsigned char)*b)<<24) | (((unsigned char)*(b + 1))<<16) | (((unsigned char)*(b + 2))<<8) | (unsigned char)*(b + 3));
}

//write value 
static int write_value(FILE *fp, char *buf){
   fwrite(buf, sizeof(buf), 1, fp); 
   fclose(fp);

   return 0;
}

//insert index
int insert_index(const char *file, char * head, char keyType, int keyLen, void * key, char *(*getKey)(const void * key, const int keyLen), char valueType, int valueLen, void * value, char *(*getValue)(const void * value, const int valueLen)){

    if(head== NULL)
        return -1;

    int count = b2i(head); //counts of HEAD
    int offset = b2i(head+ 4);// offset of Value
    int version = b2i(head + 8);//vserion of skiplist
    int index = count;

    char * startPtr = head + BASEHEADSIZE;

    int i = 0;
    for(; i<count; i++){
        if(*(startPtr + ITEMSIZE*i) == '0')
            break;
    }

    //set head
    char * headPtr = startPtr + i*ITEMSIZE;
    int  valueItemSize = keyLen + valueLen + TYPESIZE + LENSIZE;
    i2b(head + 8, ++version);

    if(i < count && (valueItemSize <= b2i(headPtr + 5))){
        *headPtr = '1';
        offset = b2i(headPtr + 1);
        index = i;
    }else{
        //init HEAD
        headPtr = startPtr + count*ITEMSIZE;
        *headPtr = '1';
        i2b(headPtr + 1, offset);// offset start From HEADSIZE
        i2b(headPtr + 5, valueItemSize); 
        
        i2b(head, ++count);
        i2b(head+ 4, offset + valueItemSize);
    }
    
    //set body
    char *bodyPos = map_file(file, b2i(headPtr + 5), offset);
    char * tmpKey = getKey(key, keyLen);
    char * tmpValue = getValue(value, valueLen);

    *bodyPos = keyType;
    i2b((bodyPos + 1), keyLen);
    memcpy(bodyPos + 5, tmpKey, keyLen); 

    *(bodyPos + 5 + keyLen) = valueType;
    i2b((bodyPos + 6 + keyLen), valueLen);
    memcpy(bodyPos + 10 + keyLen, tmpValue, valueLen); 

    free(tmpValue);
    free(tmpKey);
    sync_map(bodyPos, file);
    un_map(bodyPos, b2i(headPtr + 5));

    return index;
}

//delete index
int delete_index(char * head, int index){
    if(head == NULL)
        return -1;

    int count = b2i(head);
    int version = b2i(head + 8);

    if( index > count -1)
        return -1;

    char * startPtr = head + BASEHEADSIZE;
    char * headPtr = startPtr + index*ITEMSIZE;

    if(*headPtr == '1')
        *headPtr = '0';
    else
        return -1;

    i2b(head + 8, ++version);

    return 0;
}

//visit index
struct kvNode * visit_index(char * mapPtr, const char * file){
    int count = b2i(mapPtr);

    struct kvNode *kvList = (struct kvNode *)malloc(sizeof(struct kvNode));
    kvList->value = NULL;
    kvList->key =  NULL;
    kvList->next_kvNode = NULL;
    struct kvNode *curkvp = kvList;

    char * startPtr = mapPtr + BASEHEADSIZE;
    printf("version: %d\n", b2i(mapPtr + 8));

    for(int i = 0; i< count; i++){
        char * curPos = startPtr + i*ITEMSIZE;
        char isExist = *curPos;
        int offset = b2i(curPos + 1);
        int len = b2i(curPos + 5);

        if(isExist == '0')
            continue;
        printf("#%d: %c %d %d ->  ", i, isExist, offset, len);

        char *bodyPos = map_file(file, len, offset); 
        char keyType = *bodyPos;
        int keyLen = b2i(bodyPos + 1);
        char * key = (char *)calloc(sizeof(char), keyLen + 1);
        memcpy(key, bodyPos + 5, keyLen);

        char valueType = *(bodyPos + 5 + keyLen);
        int valueLen = b2i(bodyPos + 6 + keyLen);
        char * value = (char *)calloc(sizeof(char), valueLen + 1);
        memcpy(value, bodyPos + 10 + keyLen, valueLen);

        printf("%d %d %s : %d %d %s\n", keyType, keyLen, key, valueType, valueLen, value); 
        struct kvNode *kvp = (struct kvNode *)malloc(sizeof(struct kvNode));
        kvp->keyType = keyType;
        kvp->key = key;
        kvp->valueType = valueType;
        kvp->value = value;
        kvp->next_kvNode = NULL;
        kvp->index = i;

        curkvp->next_kvNode = kvp;
        curkvp = kvp;

       // free(key);
       // free(value);
    }

    return kvList;
}

char * init_index_file(const char *file){
    //const char * file = SKIPLISTFILE;
    if (access(file, 0)){
        if(expand_file(file, HEADSIZE) == -1)
            return NULL;

        char * mp = map_file(file, BASEHEADSIZE, 0);
        i2b(mp, 0);
        i2b(mp + 4, HEADSIZE);
        i2b(mp + 8, 0);
        sync_map(mp, file);
        un_map(mp, BASEHEADSIZE);
    }

    return map_file(file, HEADSIZE,0);
}

