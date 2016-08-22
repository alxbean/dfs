/*************************************************************************
    > File Name: struct skiplist.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Tue 12 Apr 2016 02:44:04 AM UTC
 ************************************************************************/
//#include "serial.h" //serialzation method
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*CMP)(const void *, const void *);

struct skp_node_value{
    void * value; struct skp_node_value * next_value;    
};

struct skp_node{
    int size;
    char status; //0 common, 1 new add, -1 deleted
    //int index; // the index of head item
    char keyType;
    void * key;
    char valueType;
    struct skp_node_value * value;
    int level;
    struct skp_node * next_node[0];
};

struct kv_node{
    char keyType;
    char *key;
    char valueType;
    char *value;
    struct kv_node *next_kv_node;
};

struct skiplist{
    char *name;
    //char * iFile;
    struct skp_node * head;
    int level;
    //char isRecover;
    size_t length;
    CMP cmp_key;
    CMP cmp_value;
};

//interface
struct skp_node * find_node(struct skiplist *sl, void *key);
struct kv_node* range_node(struct skiplist *sl, void *startKey, void *endKey);
struct kv_node* bigger_node(struct skiplist *sl, void *key);
struct kv_node* smaller_node(struct skiplist *sl, void *key);
void print_skiplist(struct skiplist *sl);
//struct skiplist * unserial_struct skiplist(const char * file);                      
int delete_node(struct skiplist *sl, void *key);
//int serial_skiplist(struct skiplist *sl, char *(*getKey)(const void * key, const int keyLen), char *(*getValue)(const void * value, const int valueLen));
int free_skiplist(struct skiplist * sl);

#ifdef __cplusplus
}
#endif
