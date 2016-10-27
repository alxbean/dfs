/*************************************************************************
    > File Name: logdb_map.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Thu 27 Oct 2016 10:18:43 AM CST
************************************************************************/
#include "logdb_map.h"
#include "string.h"

static struct logdb_map_node* logdb_map_node_new(void* key, void* value);
static int logdb_map_node_destroy(struct logdb_map_node** p_node);
static struct logdb_map_node* logdb_map_node_get(struct logdb_map* map, void* key);

static struct logdb_map_node* logdb_map_node_new(void* key, void* value){/*{{{*/
    struct logdb_map_node* new_node = (struct logdb_map_node*) malloc(sizeof(*new_node));
    if (NULL == new_node){
        printf("malloc new_node faild in logdb_map_node_new\n");
        return NULL;
    }

    if (NULL == key){
        printf("key is NULL in logdb_map_node_new\n");
        return NULL;
    }

    if (NULL == value){
        printf("value is NULL in logdb_map_node_new\n");
        return NULL;
    }
    
    new_node->key = key;
    new_node->value = value;
    new_node->next_node = NULL;

    return new_node;
}/*}}}*/

static int logdb_map_node_destroy(struct logdb_map_node** p_node){/*{{{*/
    if (NULL == p_node){
        printf("p_node is NULL in logdb_map_node_destroy\n");
        return -1;
    }

    struct logdb_map_node* node = *p_node;
    if (NULL == node){
        printf("node is NULL in logdb_map_node_destroy\n");
        return -1;
    }

    node->key = NULL;
    node->value = NULL;
    node->next_node = NULL;
    free(node);
    *p_node = NULL;

    return 0;
}/*}}}*/

struct logdb_map* logdb_map_new(CmpDelegate cmp){/*{{{*/
    struct logdb_map* new_map = (struct logdb_map*) malloc(sizeof(*new_map));
    if (NULL == new_map){
        printf("new_map is NULL in logdb_map_new\n");
        return NULL;
    }

    new_map->head = NULL;
    new_map->tail = NULL;
    new_map->size = 0;
    new_map->cmp = cmp;

    return new_map;
}/*}}}*/

int logdb_map_destroy(struct logdb_map** p_map){
    if (NULL == p_map){
        printf("p_map is NULL in logdb_map_destroy\n");
        return -1;
    }

    struct logdb_map* map = *p_map;
    if (NULL == map){
        printf("map is NULL in logdb_map_destroy\n");
        return -1;
    }


}

int logdb_map_insert(struct logdb_map* map, void* key, void* value){
    if (NULL == map){
        printf("map is NULL in logdb_map_insert\n");
        return -1;
    }

    if (NULL == key){
        printf("key is NULL in logdb_map_insert\n");
        return -1;
    }

    struct logdb_map_node* node = logdb_map_node_get(struct logdb_map* map, key);
}

static struct logdb_map_node* logdb_map_node_get(struct logdb_map* map, void* key){/*{{{*/
    if (NULL == map){
        printf("map is NULL in logdb_map_get\n");
        return NULL;
    }

    if (NULL == key){
        printf("key is NULL in logdb_map_get\n");
        return NULL;
    }

    struct logdb_map_node* node = map->head;

    while (node != NULL){
        if (!map->cmp(key, node->key)){
            return node;
        node = node->next_node;
    }

    return NULL;
}/*}}}*/

struct void* logdb_map_get(struct logdb_map* map, void* key){/*{{{*/
    if (NULL == map){
        printf("map is NULL in logdb_map_get\n");
        return NULL;
    }

    if (NULL == key){
        printf("key is NULL in logdb_map_get\n");
        return NULL;
    }

    struct logdb_map_node* node = map->head;

    while (node != NULL){
        if (!map->cmp(key, node->key)){
            return node->value;
        node = node->next_node;
    }

    return NULL;
}/*}}}*/


