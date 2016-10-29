/*************************************************************************
    > File Name: logdb_map.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Thu 27 Oct 2016 10:18:43 AM CST
************************************************************************/
#include "string.h"

#include "logdb_map.h"

static struct logdb_map_node* logdb_map_node_new(void* key, void* value);
static int logdb_map_node_destroy(struct logdb_map_node** p_node, FreeDelegate key_free, FreeDelegate value_free);
static struct logdb_map_node* logdb_map_pre_node_get(struct logdb_map* map, void* key);

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

static int logdb_map_node_destroy(struct logdb_map_node** p_node, FreeDelegate key_free, FreeDelegate value_free){/*{{{*/
    if (NULL == p_node){
        printf("p_node is NULL in logdb_map_node_destroy\n");
        return -1;
    }

    struct logdb_map_node* node = *p_node;
    if (NULL == node){
        printf("node is NULL in logdb_map_node_destroy\n");
        return -1;
    }

    if (NULL == key_free)
        free(node->key);
    else
        key_free(node->key);
    if (NULL == value_free)
        free(node->value)
    else
        value_free(node->value);
    node->key = NULL;
    node->value = NULL;
    node->next_node = NULL;
    free(node);
    *p_node = NULL;

    return 0;
}/*}}}*/

struct logdb_map* logdb_map_new(CmpDelegate cmp_key, FreeDelegate key_free, FreeDelegate value_free){/*{{{*/
    struct logdb_map* new_map = (struct logdb_map*) malloc(sizeof(*new_map));
    if (NULL == new_map){
        printf("new_map is NULL in logdb_map_new\n");
        return NULL;
    }

    new_map->head = NULL;
    new_map->tail = NULL;
    new_map->size = 0;
    new_map->cmp = cmp;
    new_map->key_free = key_free;
    new_map->value_free = value_free;

    return new_map;
}/*}}}*/

int logdb_map_destroy(struct logdb_map** p_map){/*{{{*/
    if (NULL == p_map){
        printf("p_map is NULL in logdb_map_destroy\n");
        return -1;
    }

    struct logdb_map* map = *p_map;
    if (NULL == map){
        printf("map is NULL in logdb_map_destroy\n");
        return -1;
    }

    struct logdb_map_node* node = map->head;
    while(node != NULL){
        map->head = node->next_node;
        if (NULL == map->head)
            map->tail = NULL;
        logdb_map_node_destroy(&node, map->key_free, map->value_free);
        node = map->head;
    }

    free(map);
    *p_map = NULL;

    return 0;
}/*}}}*/

int logdb_map_insert(struct logdb_map* map, void* key, void* value){/*{{{*/
    if (NULL == map){
        printf("map is NULL in logdb_map_insert\n");
        return -1;
    }

    if (NULL == key){
        printf("key is NULL in logdb_map_insert\n");
        return -1;
    }

    struct logdb_map_node* node = logdb_map_node_get(map, key);
    if (node != NULL){
        void* old_value = node->value;
        node->value = value;
        map->value_free(old_value);
    } else {
        struct logdb_map_node* new_node = logdb_map_node_new(key, value);
        if (NULL == new_node){
            printf("new_node is NULL in logdb_map_insert\n");
            return -1;
        }
        if (NULL == map->head){
            map->tail = new_node;
            map->head = new_node;
        } else {
            map->tail->next_node = new_node;
            map->tail = new_node;
        }
    }

    return 0;
}/*}}}*/

int logdb_map_remove(struct logdb_map* map, void* key){/*{{{*/
    struct logdb_map_node* dead_node_pre = logdb_map_pre_node_get(map, key);
    struct logdb_map_node* dead_node = NULL;
    if (NULL == dead_node_pre){
        dead_node = map->head;
        map->head = dead_node->next_node;
        logdb_map_node_destroy(&dead_node, map->key_free, map->value_free);
    } else if (NULL == dead_node_pre->next_node){
        printf("dead_node is not found in logdb_map_remove\n");
        return -1;
    } else {
        dead_node = dead_node_pre->next_node;
        dead_node_pre->next_node = dead_node->next_node;
        logdb_map_node_destroy(&dead_node, map->key_free, map->value_free);
    }

    return 0;
}/*}}}*/

struct logdb_map_node* logdb_map_node_get(struct logdb_map* map, void* key){/*{{{*/
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
        if (!map->cmp(key, node->key))
            return node;
        node = node->next_node;
    }

    return NULL;
}/*}}}*/

static struct logdb_map_node* logdb_map_pre_node_get(struct logdb_map* map, void* key){/*{{{*/
    if (NULL == map){
        printf("map is NULL in logdb_map_get\n");
        return NULL;
    }

    if (NULL == key){
        printf("key is NULL in logdb_map_get\n");
        return NULL;
    }

    struct logdb_map_node* node = map->head;
    struct logdb_map_node* pre_node = NULL;

    while (node != NULL){
        if (!map->cmp(key, node->key))
            break;
        pre_node = node;
        node = node->next_node;
    }

    return pre_node;
}/*}}}*/

void* logdb_map_get(struct logdb_map* map, void* key){/*{{{*/
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
        if (!map->cmp(key, node->key))
            return node->value;
        node = node->next_node;
    }

    return NULL;
}/*}}}*/


