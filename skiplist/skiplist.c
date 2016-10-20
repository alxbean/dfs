/*************************************************************************
    > File Name: struct skiplist.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Mon 07 Sep 2015 03:13:11 AM UTC
 ************************************************************************/
 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "skiplist.h"
#include "time.h"

#define MAX_LEVEL 16
#define NewNode(n) ((struct skp_node*)malloc(sizeof(struct skp_node)+n*sizeof(struct skp_node*)))
#define NewValue() ((struct skp_node_value*)malloc(sizeof(struct skp_node_value)))
#define FILE_NAME_LENGTH 100

//declare
static char * getKey(const void *key, const int keyLen);//example for get char * 
static char * getValue(const void *value, const int valueLen);//example for get char *

static int cmp(const void * a, const void *b);
static struct skp_node * create_node(int level , char keyType, void * key, char valueType, void * value);
static int insert_value(struct skiplist *sl, struct skp_node *node, void * value);
static struct skiplist * create_skiplist(CMP cmp_key, CMP cmp_value, const char * sl_name);
static struct skp_node * insert_node(struct skiplist * sl, char keyType, void * key, char valueType, void * value);
static struct kv_node * create_kv_node(struct skp_node *node);
static void print_line(void * vkey);
static int free_node(struct skp_node *node);
static int remove_node(struct skiplist * sl, void * key);

//default Compare fun with int 
static int cmp(const void * a, const void *b){
    return *(int *)a - *(int *)b;
}

//create a new Node of struct skiplist
static struct skp_node* create_node(int level , char keyType, void * key, char valueType, void * value){
    int nodeLevel = level+1;
    struct skp_node *np = NewNode(nodeLevel);
    if(np == NULL)
        return NULL;
    np->level = level;
    np->key = key;
    
    struct skp_node_value *nv = NewValue();
    nv->value = value;
    nv->next_value = NULL;
    np->value= nv;
    np->size = 1;
    np->status = 1;
    //np->index = -1;
    np->keyType = keyType;
    np->valueType = valueType;
    
    return np;    
}

//if the Node Already exist map with key , then insert the new value of the key into the value list
static int insert_value(struct skiplist *sl, struct skp_node *node, void * value){
    struct skp_node_value * p = node->value;    
    if(!sl->cmp_value(value, p->value))
         return 1;
    while(p->next_value != NULL){
        p = p->next_value;
        if(!sl->cmp_value(value, p->value)) 
            return 1;
    }

    struct skp_node_value * nv = NewValue();
    nv->value = value;
    nv->next_value =NULL;
    p->next_value = nv;
    node->size++;

    return 0;
}

//create a new struct skiplist
static struct skiplist * create_skiplist(CMP cmp_key, CMP cmp_value, const char * sl_name){
    struct skiplist * sl = (struct skiplist*) malloc(sizeof(struct skiplist));
    sl->name = (char *) malloc(sizeof(char) * strlen(sl_name + 1)); 
    memcpy(sl->name, sl_name, strlen(sl_name));
    if(sl == NULL)
        return NULL;
    sl->level = 0; 
    sl->cmp_key = cmp_key;
    sl->cmp_value = cmp_value;
    //sl->isRecover = isRecover;
    //sl->iFile = init_index_file(sl->name); 
    
    struct skp_node * head = create_node(MAX_LEVEL, 0, NULL, 0, NULL);
    if(head == NULL){
        free(sl);
        return NULL;
    }

    sl->head = head;
    for(int i = 0; i<MAX_LEVEL; i++){
        sl->head->next_node[i] = NULL;
    }

    sl->length= 0;
    srand(time(0));

    return sl;
}

//generate a random value as the Node level of struct skiplist
int random_level(){
    int level = 0;
    while(rand()%2){
        level++;
    }

    return level%MAX_LEVEL;
}

//insert a Node to struct skiplist
static struct skp_node* insert_node(struct skiplist * sl, char keyType, void * key, char valueType, void * value){
    if(sl == NULL)
        return NULL;
    struct skp_node *update[MAX_LEVEL];
    struct skp_node *q = NULL, *p= sl->head;

    for(int i = sl->level; i>=0; i--){
        while(((q = p->next_node[i])&& (sl->cmp_key(key,  q->key) > 0)))
            p = q;
        update[i] = p;
    }

    if(q&&!sl->cmp_key(key, q->key)){
        if(insert_value(sl, q, value)== 1)
            return NULL;
        return 0;
    }

    int level = random_level();
    if(level > sl->level){
        for(int i=sl->level+1; i <= level; i++) update[i] = sl->head;
        sl->level = level;
    }
    
    struct skp_node *node = create_node(level, keyType, key, valueType, value);
    for(int i = level; i >=0; i--){
        node->next_node[i] = update[i]->next_node[i];
        update[i]->next_node[i] = node;
    }


    sl->length++;

    return node;
}

static struct kv_node * create_kv_node(struct skp_node *node){
    if(node == NULL)
        return NULL;
    struct kv_node* pnode = (struct kv_node*)malloc(sizeof(struct kv_node));
    pnode->keyType = node->keyType;
    pnode->key = (char *)node->key;
    pnode->valueType = node->valueType;
    pnode->value = (char *)node->value;
    pnode->next_kv_node= NULL; 

    return pnode;
}

/*
 *Search Operation
*/


//find the Node from struct skiplist by the key
struct skp_node * find_node(struct skiplist *sl, void *key){
    struct skp_node *q = NULL, *p = sl->head;
    for(int i = sl->level; i>=0; i--){
        while((q=p->next_node[i]) && (sl->cmp_key(key , q->key) > 0))
            p = q;
        if(q && !sl->cmp_key(key , q->key) && q->status != -1)
            return q;
    }

    return NULL;
}

//find the Nodes by the range key from struct skiplist
struct kv_node* range_node(struct skiplist *sl, void *startKey, void *endKey){
    struct skp_node *p = sl->head; 
    struct kv_node *kvHead = create_kv_node(p);
    struct kv_node *tmp_kvNode = kvHead;

    while((p = p->next_node[0]) && (sl->cmp_key(startKey, p->key) > 0))
    ;//empty body

    while( p &&  (sl->cmp_key(endKey, p->key) >= 0) && p->status != -1){
        tmp_kvNode->next_kv_node = create_kv_node(p);
        tmp_kvNode = tmp_kvNode->next_kv_node;
        p = p->next_node[0];
    }

    return kvHead;
}

//find the bigger Node of the key
struct kv_node* bigger_node(struct skiplist *sl, void *key){
    struct skp_node *p = sl->head;

    while(p->next_node[0]){
        p = p->next_node[0];
        if(sl->cmp_key(key, p->key) < 0 && p->status != -1){
             struct kv_node *tmp_kvNode = create_kv_node(p);
             return tmp_kvNode; 
        }
    }

    return NULL;
}

//find the smaller Node of the key
struct kv_node* smaller_node(struct skiplist *sl, void *key){
    struct skp_node *p = sl->head, *q = NULL;

    while(p->next_node[0]){
        p = p->next_node[0];
        q = p->next_node[0];

        if( (sl->cmp_key(key, p->key) > 0) &&((!q) || (sl->cmp_key(key, q->key) <= 0)) && p->status != -1){
             struct kv_node *tmp_kvNode = create_kv_node(p);
             return tmp_kvNode; 
        }
    }

    return NULL;
}


//print '-' with different length of key
static void print_line(void * vkey){
    int key = strlen((char *)vkey);
    printf("---");
    while(key--)
        printf("-");
    printf("-");
}

//print struct skiplist with '-----'
void print_skiplist(struct skiplist *sl){
    int level = sl->level;

    printf("Level: %d\n",level);
    for(int i = 0; i<=level; i++){
        struct skp_node * p = sl->head;
        struct skp_node * q = NULL;
        printf("Head ");
        while(p->next_node[0]){
            q = p->next_node[0];
            if(q->level >= i &&q->status != -1) 
                printf("-> %s ",(char*)q->key);
            else
                print_line(q->key);
            p = q;
        }
        printf("-> NULL \n");
    }
}


//struct skiplist * unserial_skiplist(const char * file){                        /*{{{*/
//    struct skiplist * sl = create_skiplist(cmp, cmp, 0, file);
//    struct kvNode * kvList = visit_index(sl->iFile, sl->name);
//
//    while(kvList->next_kv_node){
//        struct kvNode * node = kvList->next_kv_node;
//        printf("%s %s \n", node->key, node->value);
//        Node *NewNode = insert_node(sl, node->keyType, node->key, node->valueType, node->value);
//        NewNode->index = node->index;
//         
//        kvList = kvList->next_kv_node;
//    }
//    
//    sl->isRecover = 1;
//    free(kvList);
//    return sl;
//}/*}}}*/

//free Node
static int free_node(struct skp_node *node){
    if(node == NULL)
        return -1;
    struct skp_node_value * vp = node->value;
    while(vp){
        struct skp_node_value * tvp = vp;
        vp = vp->next_value;
        free(tvp->value);
        free(tvp);
    }

    free(node->key);
    free(node);

    return 0;
}

//delete the Node from struct skiplist
static int remove_node(struct skiplist * sl, void * key){
    struct skp_node * update[MAX_LEVEL];
    struct skp_node * q = find_node(sl, key);
    if(q == NULL)
        return -1;
    for(int i = sl->level; i>=0; i--){
        if(update[i]->next_node[i] == q){
            update[i]->next_node[i] = q->next_node[i];
            if(sl->head->next_node[i] == NULL)
                sl->level--;
        }
    }

    free_node(q);
    q = NULL;
    sl->length--;

    return 0;
}

//mark the node to delete
int delete_node(struct skiplist *sl, void *key){
    struct skp_node * q = find_node(sl, key);
    if(q == NULL)
        return -1;
    q->status = -1;

    return 0;
}

//serialzation struct skiplist
//int serial_skiplist(struct skiplist *sl, char *(*getKey)(const void * key, const int keyLen), char *(*getValue)(const void * value, const int valueLen)){/*{{{*/
//    Node * p = sl->head;
//    Node * q = NULL;
//    char * head = sl->iFile;      
//    
//    while(p->next_node[0]){
//        q = p->next_node[0];
//        char keyType = q->keyType;
//        void * key = q->key;
//        char valueType = q->valueType;
//        void * value = q->value->value;
//
//        int keyLen = strlen((char*)key);// temple: assume key type is char *
//        int valueLen = strlen((char*)value);//temple: assume value is char * 
//
//        switch(q->status){
//            case 1:
//                    {
//                        int index = insert_index(sl->name, head, keyType, keyLen, key, getKey, valueType, valueLen, value, getValue);  
//                        if( index != -1){
//                          q->index = index; 
//                          q->status = 0;
//                        }
//                    }
//                    break;
//            
//            case -1:
//                    {
//                        if(q->index != -1){
//                            delete_index(head, q->index);
//                            remove_node(sl, q);
//                        }
//                    }
//                    break;
//            default:
//                    break;
//        }
//        p = q;
//    }
//
//    return 0;
//}/*}}}*/

//free the struct skiplist
int free_skiplist(struct skiplist * sl){
    if(!sl)
        return -1;
    struct skp_node *tmpNP = sl->head->next_node[0];
    while(tmpNP){
        struct skp_node * tmpNode = tmpNP;
        tmpNP = tmpNP->next_node[0];

        free_node(tmpNode);
    }

    free(sl->head);
    free(sl);
    return 0;
}

static char * getKey(const void *key, const int keyLen){
    char * buf = (char *)malloc(sizeof(char)*keyLen);
    strncpy(buf, (char*) key, keyLen);

    return buf;
}

static char * getValue(const void *value, const int valueLen){
    char * buf = (char *)malloc(sizeof(char)*valueLen);
    strncpy(buf, (char*) value, valueLen);
    
    return buf;
}

int main(){
      //struct skiplist * sl = unserial_skiplist("./index/test.idx");
    struct skiplist * sl = create_skiplist(cmp, cmp, "test");
    clock_t start, finish;
    start = clock();

    for (int i = 0; i < 1000000; i++){
        int x = (rand()%1000000) + 1;
        //if (i%1000 == 0)
        //   printf("x: %d\n", x);
        int *key = (int *)malloc(sizeof(int));
        int *value = (int *)malloc(sizeof(int));
        *key = x;
        *value = x;
        insert_node(sl, 0, key, 0, value);
    }

    finish = clock();
    printf("time cost: %f s\n", (double)(finish - start)/CLOCKS_PER_SEC);

    //char key[] = "666";
    //char value[] = "fff";
    //insert_node(sl, 0, key, 0, value); 
    //char key1[] = "222";
    //char value1[] = "bb";
    //insert_node(sl, 0, key1, 0, value1);
    //char key2[] = "333";
    //char value2[] = "cc";
    //insert_node(sl, 0, key2, 0, value2);
    //
    //print_skiplist(sl);
//    char startKey[] = "000";
//    char endKey[] = "111";
//
//    simpleNode *sn = rangeNode(sl, startKey , endKey);
//    while(sn->next_simpleNode){
//        sn = sn->next_simpleNode; 
//        printf("%s : %s\n", sn->key, sn->value->value);
//    }
//    char delKey[] = "111";
//    deleteNode(sl, delKey);
//    serialskiplist(sl);
//    printskiplist(sl);
//    Node *node = findNode(sl, &key);
//    if(node == NULL)
//        printf("key:%d NOT FIND~!\n", key);
//    else{
//        Value * pv = node->head;
//        while(pv != NULL){
//            printf("the value: %d mapping key: %d \n", *(int *)pv->value, *(int *)node->key);
//            pv = pv->next_value;
//        }
//    }
//    
//    if(deleteNode(sl, &key) == 0)
//        printf("the node %d is deleted \n",key);
//    printskiplist(sl);
//
//    char const * file = "sl.data";
//    serialSL(sl, file);
 //   struct skiplist * sl = unserialSL(file);
    //printf("----------------query----------\n");
    //char k[] = "333";

    //struct skp_node * node = find_node(sl, k);
    //printf("%s\n", (char *) node->value->value); 
    //char delKey[] = "333";
    //delete_node(sl, delKey);
    //print_skiplist(sl);
    //node = find_node(sl, k);
    //if(NULL == node)
    //    printf("not found~!\n");
    //else
    //    printf("%s\n", (char *) node->value->value); 
}

