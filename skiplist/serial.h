/************************************************************************
    > File Name: serial.h
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
#define BASEHEADSIZE 12//count(int) + offset(int) + version(int)
#define HEADSIZE 4096//256M
#define ITEMSIZE 9 //Head Item size: char + int + int
#define TYPESIZE 8 //keyType: int + valueType: int 
#define LENSIZE 8 // keyLen: int + valueLen: int

typedef struct kvNode{
    char keyType;
    char *key;
    char valueType;
    char *value;
    int index;
    struct kvNode *next_kvNode;
} kvNode;

/*Mapped Init*/
//insert index
int insert_index(const char *file, char * head, char keyType, int keyLen, void * key, char *(*getKey)(const void * key, const int keyLen), char valueType, int valueLen, void * value, char *(*getValue)(const void * value, const int valueLen));

//delete index
int delete_index(char * head, int index);

//visit index
struct kvNode * visit_index(char * mapPtr, const char * file);

char * init_index_file(const char * file);
