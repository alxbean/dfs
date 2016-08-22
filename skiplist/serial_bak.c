/************************************************************************
    > File Name: serial.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Tue 15 Sep 2015 02:37:28 AM UTC
 ************************************************************************/

 #include <stdio.h>
 #include <stdlib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/mman.h>
 #include <time.h>
 #include <error.h>
 #include <string.h>

#define MAXSIZE 100
#define HEADSIZE 4096//256M
#define ITEMSIZE 9 //Head Item size: char + int + int
#define TYPESIZE 8 //keyType: int + valueType: int 
#define LENSIZE 8 // keyLen: int + valueLen: int
#define SKIPLISTFILE "./persistence/skiplist.data"

/*Mapped Init*/

//get FileSize
long getFileSize(const char * file){
    struct stat sb;

    if ( stat(file, &sb) == -1){
        perror("stat");
    }

    return sb.st_size;
}

//increase FileSize
int expandFile(const char * file , int size){
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
int syncMap(char * mapped){
    
    if ((msync((void *) mapped, getFileSize(SKIPLISTFILE), MS_SYNC)) == -1){
        return -1;
    }

    return 0;
}

//unMap
int unMap(char * mapped, int size){
    if ((munmap((void *)mapped, size)) == -1){
        return -1;
    }

    return 0;
}

//map 
char *mapMaster(const char * file, long size, long offset){
    int fd;
    char * mapped;
    long fileSize = getFileSize(file);
    
    if( (fileSize - offset) < size)
        expandFile(file, size*2);

    int pageSize = sysconf(_SC_PAGE_SIZE);
    int align_offset = (offset/pageSize)*pageSize;
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
void i2b(char * a, int b){
    *a++ = (b>>24) & 0xFF;
    *a++ = (b>>16) & 0xFF;
    *a++ = (b>>8)  & 0xFF;
    *a++ = b & 0xFF;
}

//byte 2 int
int b2i(char *b){
    return ((((unsigned char)*b)<<24) | (((unsigned char)*(b + 1))<<16) | (((unsigned char)*(b + 2))<<8) | (unsigned char)*(b + 3));
}

//write value 
int writeValue(FILE *fp, char *buf){
   fwrite(buf, sizeof(buf), 1, fp); 
   fclose(fp);

   return 0;
}

//insert index
int insertIndex(char * head, char keyType, int keyLen, void * key, char *(*getKey)(const void * key, const int keyLen), char valueType, int valueLen, void * value, char *(*getValue)(const void * value, const int valueLen)){

    if(head== NULL)
        return -1;

    int count = b2i(head); //counts of HEAD
    int offset = b2i(head+ 4);// offset of Value

    int i = 0;
    char * startPtr = head + 8;

    for(i=0; i<count; i++){
        if(*(startPtr + ITEMSIZE*i) == '0')
            break;
    }

    //set head
    char * headPtr = startPtr + i*ITEMSIZE;
    int  valueItemSize = keyLen + valueLen + TYPESIZE + LENSIZE;

    if(i < count && (valueItemSize <= b2i(headPtr + 5))){
        *headPtr = '1';
        offset = b2i(headPtr + 1);
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
    char *bodyPos = mapMaster(SKIPLISTFILE, b2i(headPtr + 5), offset);
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
    syncMap(bodyPos);
    unMap(bodyPos, b2i(headPtr + 5));

    return 0;
}

//delete index
int deleteIndex(char * head, int index){
    if(head == NULL)
        return -1;

    int count = b2i(head);

    if( index > count -1)
        return -1;

    char * startPtr = head + 8;
    char * headPtr = startPtr + index*ITEMSIZE;

    if(*headPtr == '1')
        *headPtr = '0';
    else
        return -1;

    return 0;

}

//visit index
void visitIndex(char * mapPtr){
    int count = b2i(mapPtr);
    int i = 0;

    char * startPtr = mapPtr + 8;

    for(i = 0; i< count; i++){
        char * curPos = startPtr + i*ITEMSIZE;
        char isExist = *curPos;
        int offset = b2i(curPos + 1);
        int len = b2i(curPos + 5);

        printf("#%d: %c %d %d ->  ", i, isExist, offset, len);

        char *bodyPos = mapMaster(SKIPLISTFILE, len, offset); 

        char keyType = *bodyPos;
        int keyLen = b2i(bodyPos + 1);
        char * key = (char *)calloc(sizeof(char), keyLen + 1);
        memcpy(key, bodyPos + 5, keyLen);

        char valueType = *(bodyPos + 5 + keyLen);
        int valueLen = b2i(bodyPos + 6 + keyLen);
        char * value = (char *)calloc(sizeof(char), valueLen + 1);
        memcpy(value, bodyPos + 10 + keyLen, valueLen);

        printf("%d %d %s : %d %d %s\n", keyType, keyLen, key, valueType, valueLen, value); 

        free(key);
        free(value);
    }
}

//callBack
char * getKey(const void *key, const int keyLen){
    char * buf = (char *)malloc(sizeof(char)*keyLen);
    memcpy(buf, (char*) key, keyLen);

    return buf;
}

char * getValue(const void *value, const int valueLen){
    char * buf = (char *)malloc(sizeof(char)*valueLen);
    memcpy(buf, (char*) value, valueLen);
    
    return buf;
}

char * init_skiplist_data(){
    const char * file = SKIPLISTFILE;

    if (access(file, 0)){
        if(expandFile(file, HEADSIZE) == -1)
            return NULL;

        char * mp = mapMaster(file, 8, 0);
        i2b(mp, 0);
        i2b(mp + 4, HEADSIZE);
        syncMap(mp);
        unMap(mp, 8);
    }
    
    return mapMaster(file, HEADSIZE, 0);
}

int main(){
    char * head = init_skiplist_data();

//    char keyType = 5;
//    int keyLen = 3;
//    char key [] = "005";
//    char valueType = 1;
//    int valueLen = 5;
//    char value[] = "world";
//
//    insertIndex(head, keyType, keyLen, key, getKey, valueType, valueLen, value, getValue);

     visitIndex(head);
//    int k = deleteIndex(head, 7);
//    if(k == -1)
//        printf("delete failed");
//    else{
//        printf("--------------------------\n");
//        visitIndex(head);
//    }

    syncMap(head);
    unMap(head, HEADSIZE);
    
    return 0;
}

