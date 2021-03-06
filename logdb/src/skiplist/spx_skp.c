/*************************************************************************
> File Name: struct spx_skp.c
> Author: shuaixiang
> Mail: shuaixiang@yuewen.com
> Created Time: Mon 07 Sep 2015 03:13:11 AM UTC
************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "spx_skp.h"

#define SpxSkpMaxLevel 16 
#define NewNode(n) ((struct spx_skp_node *) calloc(1, sizeof(struct spx_skp_node) + n*sizeof(struct spx_skp_node*)))
#define NewValue() ((struct spx_skp_node_value*) calloc(1, sizeof(struct spx_skp_node_value)))

//declare
static struct spx_skp_node * spx_skp_node_new(int level , void * key, void * value);
static int spx_skp_value_insert(struct spx_skp *skp, struct spx_skp_node *node, void * value);
static void spx_skp_line_print(void * vkey);
static int spx_skp_node_free(struct spx_skp *skp, struct spx_skp_node *node);
static void spx_skp_line_print(void * vkey);
static int spx_skp_level_rand();
static void spx_skp_default_free(void * pfree);

//generate a random value as the Node level of struct spx_skp
static int spx_skp_level_rand()/*{{{*/
{
    int level = 0;
    while(rand() % 2){
        level++;
    }
    return (level % SpxSkpMaxLevel);
}/*}}}*/

//create a new Node of struct spx_skp
static struct spx_skp_node * spx_skp_node_new(int level , void * key, void * value)/*{{{*/
{
    int node_level = level + 1;
    struct spx_skp_node *np = NewNode(node_level);
    if(np == NULL)
        return NULL;
    np->level = level;
    np->key = key;

    //init Value    
    struct spx_skp_node_value *nv = NewValue();
    nv->status = NEWADD;
    nv->value = value;
    nv->next_value = NULL;
    nv->index = -1;

    //init Node
    np->value= nv;
    np->size = 1;
    //np->key_type = key_type;
    //np->value_type = value_type;

    return np;    
}/*}}}*/

//if the Node Already exist map with key , then insert the new value of the key into the value list
static int spx_skp_value_insert(struct spx_skp *skp, struct spx_skp_node *node, void * value)/*{{{*/
{
    struct spx_skp_node_value * pv = node->value;    
    if ( !skp->cmp_value(value, pv->value) )
        return 1;
    while (pv->next_value != NULL){
        pv = pv->next_value;
        if (!skp->cmp_value(value, pv->value)) 
            return 1;
    }

    struct spx_skp_node_value * nv = NewValue();
    nv->value = value;
    nv->status = NEWADD;
    nv->index = -1;
    nv->next_value =NULL;
    pv->next_value = nv;
    node->size++;

    return 0;
}/*}}}*/

//free Node
static int spx_skp_node_free(struct spx_skp *skp, struct spx_skp_node *node)/*{{{*/
{
    if (NULL == skp){
        printf("spx_skp_node free skp is NULL\n");
        return -1;
    }

    if (NULL == node){
        printf("spx_skp_node free node is NULL\n");
        return -1;
    }

    struct spx_skp_node_value * vp = node->value;
    while(vp){
        struct spx_skp_node_value * fvp = vp;
        vp = fvp->next_value;
        if (skp->is_free_kv == true)
            skp->free_value(fvp->value);//do not free value pointer, if it is needed, a free_method should be recall
        free(fvp);// free spx_skp_node_value
    }

    if (skp->is_free_kv == true)
        skp->free_key(node->key);//do not free key pointer, if it is need, a free_method should be recall
    free(node);// free skp_node

    return 0;
}/*}}}*/

//delete the Node from struct spx_skp
int spx_skp_node_remove(struct spx_skp * skp, struct spx_skp_node *q)/*{{{*/
{
    struct spx_skp_node * update[SpxSkpMaxLevel];
    if (q == NULL)
        return -1;
    int i;
    for(i = skp->level; i>=0; i--){
        if(update[i]->next_node[i] == q){
            update[i]->next_node[i] = q->next_node[i];
            if(skp->head->next_node[i] == NULL)
                skp->level--;
        }
    }

    spx_skp_node_free(skp, q);
    q = NULL;
    skp->length--;

    return 0;
}/*}}}*/

//print '-' with different length of key
static void spx_skp_line_print(void * vkey)/*{{{*/
{
    int key = strlen((char *)vkey);
    printf("---");
    while(key--)
    printf("-");
    printf("-");
}/*}}}*/

//defautl_free method
static void spx_skp_default_free(void * pfree)/*{{{*/
{
    free(pfree);
}/*}}}*/




/*
*spx_skp basic operation
*/

//create a tmp spx_skp ,key and value will not be free when destory is called 
struct spx_skp * spx_skp_new_tmp(SpxSkpCmpDelegate cmp_key, SpxSkpCmpDelegate cmp_value, const char * skp_name, SpxSkpFreeDelegate free_key, SpxSkpFreeDelegate free_value){/*{{{*/
    struct spx_skp * skp = (struct spx_skp*) malloc(sizeof(struct spx_skp));
    if(skp == NULL){
        return NULL;
    }

    skp->name = (char *) malloc(sizeof(char) * (strlen(skp_name) + 1));
    strncpy(skp->name, skp_name, strlen(skp_name));
    *(skp->name + strlen(skp_name)) = '\0';

    skp->level = 0; 
    skp->cmp_key = cmp_key;
    skp->cmp_value = cmp_value;
    skp->is_free_kv = false;
    if (NULL == free_key){
        skp->free_key = spx_skp_default_free;
    } else {
        skp->free_key = free_key;
    }

    if (NULL == free_value){
        skp->free_value = spx_skp_default_free;
    } else {
        skp->free_value = free_value;
    }

    struct spx_skp_node * head = spx_skp_node_new(SpxSkpMaxLevel, NULL, NULL);
    if(head == NULL){
        free(skp);
        return NULL;
    }

    int i;
    for(i = 0; i < SpxSkpMaxLevel; i++){
        head->next_node[i] = NULL;
    }

    skp->head = head;

    skp->length= 0;
    srand(time(0));

    return skp;
}/*}}}*/

//create a new struct spx_skp
struct spx_skp * spx_skp_new(SpxSkpCmpDelegate cmp_key, SpxSkpCmpDelegate cmp_value, const char * skp_name, SpxSkpFreeDelegate free_key, SpxSkpFreeDelegate free_value)/*{{{*/
{
    struct spx_skp * skp = (struct spx_skp*) malloc(sizeof(struct spx_skp));
    if(skp == NULL){
        return NULL;
    }

    skp->name = (char *) malloc(sizeof(char) * (strlen(skp_name) + 1));
    strncpy(skp->name, skp_name, strlen(skp_name));
    *(skp->name + strlen(skp_name)) = '\0';

    skp->level = 0; 
    skp->cmp_key = cmp_key;
    skp->cmp_value = cmp_value;
    skp->is_free_kv = true;
    if (NULL == free_key){
        skp->free_key = spx_skp_default_free;
    } else {
        skp->free_key = free_key;
    }

    if (NULL == free_value){
        skp->free_value = spx_skp_default_free;
    } else {
        skp->free_value = free_value;
    }

    struct spx_skp_node * head = spx_skp_node_new(SpxSkpMaxLevel, NULL, NULL);
    if(head == NULL){
        free(skp);
        return NULL;
    }

    int i;
    for(i = 0; i < SpxSkpMaxLevel; i++){
        head->next_node[i] = NULL;
    }

    skp->head = head;

    skp->length= 0;
    srand(time(0));

    return skp;
}/*}}}*/

//free the struct spx_skp
int spx_skp_destory(struct spx_skp * skp)/*{{{*/
{
    if(!skp)
        return -1;
    struct spx_skp_node *tmp_skp_node = skp->head;
    while(tmp_skp_node){
        struct spx_skp_node * free_skp_node = tmp_skp_node;
        tmp_skp_node = tmp_skp_node->next_node[0];
        spx_skp_node_free(skp, free_skp_node);
    }

    free(skp->name);
    free(skp);
    return 0;
}/*}}}*/

//insert a Node to struct spx_skp
struct spx_skp_node * spx_skp_insert(struct spx_skp * skp, void * key, void * value)/*{{{*/
{
    if (skp == NULL){
        printf("skp is NULL\n");
        return NULL;
    }

    struct spx_skp_node *update[SpxSkpMaxLevel];
    struct spx_skp_node *cur_node = NULL, *pre_node = skp->head;

    int i;
    //find the nodes to insert after
    for(i = skp->level; i >= 0; i--){
        while ((cur_node = pre_node->next_node[i]) && (cur_node->key != NULL) && (skp->cmp_key(key , cur_node->key) > 0))
            pre_node = cur_node; 
        update[i] = pre_node;
    }

    //exist node 
    if ((cur_node != NULL) && (cur_node->key != NULL) && !skp->cmp_key(key, cur_node->key)){
        if(spx_skp_value_insert(skp, cur_node, value) == 1)//value exist
            return NULL;
        return cur_node;
    }

    int level = spx_skp_level_rand();
    if (level > skp->level)
    {//add level
        for(i = skp->level + 1; i <= level; i++) update[i] = skp->head;
        skp->level = level;
    }

    struct spx_skp_node *node = spx_skp_node_new(level, key, value);
    for(i = level; i >= 0; i--){
        node->next_node[i] = update[i]->next_node[i];
        update[i]->next_node[i] = node;
    }

    skp->length++;

    return node;
}/*}}}*/

//find the Node from struct spx_skp by the key
struct spx_skp_node * spx_skp_find(struct spx_skp *skp, void *key)/*{{{*/
{
    struct spx_skp_node *cur_node = NULL, *pre_node = skp->head;
    int i;
    for(i = skp->level; i>=0; i--){
        while ( (cur_node = pre_node->next_node[i]) && (cur_node->key != NULL) && (skp->cmp_key(key , cur_node->key) > 0))
            pre_node = cur_node; 
        if(cur_node && !skp->cmp_key(key , cur_node->key) && cur_node->size != 0) // if q->size = 0, this node is DELETED
            return cur_node;
    }

    return NULL;
}/*}}}*/

//mark the node to delete
int spx_skp_delete(struct spx_skp *skp, void *key)/*{{{*/
{
    struct spx_skp_node* d_node = spx_skp_find(skp, key);
    if (d_node == NULL)
        return -1;
    d_node->size = 0;
    struct spx_skp_node_value *v = d_node->value;
    while (v){
        v->status = DELETED;
        v = v->next_value;
    }

    return 0;
}/*}}}*/

//print struct spx_skp with '-----'
//void spx_skp_print(struct spx_skp *skp)/*{{{*/
//{
//    int level = skp->level;
//    printf("Level: %d\n",level);
//    int i;
//    for(i = 0; i<=level; i++){
//        struct spx_skp_node * cur_node = skp->head->next_node[0];
//        printf("Head ");
//        while(cur_node){
//            if(cur_node->level >= i && cur_node->size != 0){
//                if (SKP_TYPE_STR == cur_node->key_type)
//                    printf("-> %s ",(char*)cur_node->key);
//                else if (SKP_TYPE_INT == cur_node->key_type)
//                    printf("-> %d ", *(int *)cur_node->key);
//                else if (SKP_TYPE_LONG == cur_node->key_type)
//                    printf("-> %ld ", *(int64_t*)cur_node->key);
//                else
//                    printf("-> unknow");
//            }
//            else
//                spx_skp_line_print(cur_node->key);
//             cur_node = cur_node->next_node[0];
//        }
//        printf("-> NULL \n");
//    }
//}/*}}}*/

/*
* spx_skp query operation
*/

// query result struction
struct spx_skp_value_list * spx_skp_value_list_new()/*{{{*/
{
    struct spx_skp_value_list * value_list = (struct spx_skp_value_list *) malloc(sizeof(struct spx_skp_value_list));
    if (NULL == value_list){
        printf("malloc spx_skp_value_list failed\n");
        return NULL;
    }

    value_list->head = NULL;
    value_list->tail = NULL;

    return value_list;
}/*}}}*/

int spx_skp_value_list_insert(struct spx_skp_value_list *value_list, struct spx_skp_node_value * value)/*{{{*/
{
    if (NULL == value){
        printf("value is NULL\n");
        return -1;
    }
    struct spx_skp_value_list_node * vp = (struct spx_skp_value_list_node*) malloc(sizeof(struct spx_skp_value_list_node));
    if (NULL == vp){
        printf("malloc spx_skp_node_value failed\n");
        return -1;
    }

    vp->value = value;
    vp->next_value_list_node = NULL;

    if ( NULL == value_list->tail){
        value_list->head = vp;
        value_list->tail = vp;
    } else {
        value_list->tail->next_value_list_node = vp;
        value_list->tail = vp;
    }

    return 0;
}/*}}}*/

void spx_skp_value_list_free(struct spx_skp_value_list *value_list)/*{{{*/
{
    struct spx_skp_value_list_node *vp = value_list->head;

    while(vp){
        struct spx_skp_value_list_node *fvp = vp;
        vp = vp->next_value_list_node;
        //value can not be free
        free(fvp);
    } 

    free(value_list);
}/*}}}*/


//find the node by the key
struct spx_skp_value_list* spx_skp_query(struct spx_skp *skp, void *key)/*{{{*/
{ 
    if (NULL == key){
        printf("key is NULL\n");
        return NULL;
    }
    struct spx_skp_node *cur_node = NULL, *pre_node =  skp->head;
    struct spx_skp_value_list * value_list = spx_skp_value_list_new();
    int rc = -1;

    int i;
    for(i = skp->level; i>=0; i--){
        while ( (cur_node = pre_node->next_node[i]) && (cur_node->key != NULL) && ( (rc = skp->cmp_key(key , cur_node->key)) > 0))
            pre_node = cur_node; 
        if ( !rc && (cur_node->size != 0))
        {   // if q->size = 0, this node is DELETED
            struct spx_skp_node_value * tmp_nv = cur_node->value; 
            while (tmp_nv != NULL){
                spx_skp_value_list_insert(value_list, tmp_nv);
                tmp_nv = tmp_nv->next_value;
            }
            break;
        }
    }

    return value_list;
}/*}}}*/

//find the Nodes by the range key from struct spx_skp
struct spx_skp_value_list* spx_skp_range_query(struct spx_skp *skp, void *start_key, void *end_key)/*{{{*/
{
    if (NULL == start_key || NULL == end_key){
        printf("start_key or end_key is NULL\n");
        return NULL;
    }

    struct spx_skp_node *cur_node = skp->head; 
    struct spx_skp_value_list * value_list = spx_skp_value_list_new();

    while( (cur_node = cur_node->next_node[0]) && (cur_node->key != NULL) && (skp->cmp_key(start_key, cur_node->key) > 0))
    ;//empty body

    while( (cur_node != NULL) && ((cur_node->key != NULL) && skp->cmp_key(end_key, cur_node->key) >= 0) && cur_node->size !=0 ){
        struct spx_skp_node_value * tmp_nv = cur_node->value;
        while (tmp_nv != NULL){
            spx_skp_value_list_insert(value_list, tmp_nv);
            tmp_nv = tmp_nv->next_value;
        }
        cur_node = cur_node->next_node[0];
    }

    return value_list;
}/*}}}*/

//find the bigger Node of the key
struct spx_skp_value_list* spx_skp_bigger_near_query(struct spx_skp *skp, void *key)/*{{{*/
{
    if (NULL == key){
        printf("key is NULL\n");
        return NULL;
    }

    struct spx_skp_node *cur_node = skp->head->next_node[0];
    struct spx_skp_value_list * value_list = spx_skp_value_list_new();

    while (cur_node){
        if ( (cur_node->key != NULL) && skp->cmp_key(key, cur_node->key) <= 0 && cur_node->size != 0){
            struct spx_skp_node_value * tmp_nv = cur_node->value; 
            while (tmp_nv != NULL){
                spx_skp_value_list_insert(value_list, tmp_nv->value);
                tmp_nv = tmp_nv->next_value;
            }
            break;
        }

        cur_node = cur_node->next_node[0];
    }

    return value_list;
}/*}}}*/

//find the bigger Nodes of the key
struct spx_skp_value_list* spx_skp_bigger_query(struct spx_skp *skp, void *key)/*{{{*/
{
    struct spx_skp_node *cur_node = skp->head->next_node[0];
    struct spx_skp_value_list * value_list = spx_skp_value_list_new();

    while(cur_node){
        if( (cur_node->key !=NULL) && skp->cmp_key(key, cur_node->key) <= 0 && cur_node->size != 0)
        {//if p->size = 0, it has been deleted
         struct spx_skp_node_value * tmp_nv = cur_node->value; 
         while (tmp_nv != NULL){
             spx_skp_value_list_insert(value_list, tmp_nv->value);
             tmp_nv = tmp_nv->next_value;
        }
        }

        cur_node = cur_node->next_node[0];
    }

    return value_list;
}/*}}}*/

//find the smaller Node of the key
struct spx_skp_value_list* spx_skp_smaller_near_query(struct spx_skp *skp, void *key)/*{{{*/
{
    if (NULL == key){
        printf("key is NULL\n");
        return NULL;
    }

    struct spx_skp_node *next_node = NULL, *cur_node = skp->head->next_node[0]; 
    struct spx_skp_value_list * value_list = spx_skp_value_list_new();

    while (cur_node){
        next_node = cur_node->next_node[0];

        if( (cur_node->key != NULL) && (skp->cmp_key(key, cur_node->key) >= 0) && ( ((next_node->key != NULL) && (skp->cmp_key(key, next_node->key) <= 0)) || (NULL == next_node)) ){
            struct spx_skp_node_value * tmp_nv = cur_node->value; 
            while (tmp_nv != NULL){
                spx_skp_value_list_insert(value_list, tmp_nv->value);
                tmp_nv = tmp_nv->next_value;
            }
            break;
        }

        cur_node = next_node;
    }

    return value_list;
}/*}}}*/

//find the smaller Nodes of the key
struct spx_skp_value_list* spx_skp_smaller_query(struct spx_skp *skp, void *key)/*{{{*/
{
    if (NULL == key){
        printf("key is NULL\n");
        return NULL;
    }

    struct spx_skp_node *cur_node = skp->head->next_node[0]; 
    struct spx_skp_value_list * value_list = spx_skp_value_list_new();

    while(cur_node){
        if( (cur_node->key != NULL) && (skp->cmp_key(key, cur_node->key) >= 0)){
            struct spx_skp_node_value * tmp_nv = cur_node->value; 
            while (tmp_nv != NULL){
                spx_skp_value_list_insert(value_list, tmp_nv->value);
                tmp_nv = tmp_nv->next_value;
            }
        } else{
            break;
        }

        cur_node = cur_node->next_node[0];
    }

    return value_list;
}/*}}}*/



