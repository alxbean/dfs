/*************************************************************************
    > File Name: logdb_map.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Thu 20 Oct 2016 10:36:55 AM CST
 ************************************************************************/

#ifndef _LOGDB_MAP_H_
#define _LOGDB_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif
    #include <stdio.h>
    #include <stdlib.h>
    #include "logdb_types.h"

    struct logdb_map_node {
        void* key;
        void* value;
        struct logdb_map_node* next_node;
    };

    struct logdb_map {
        size_t size;
        struct logdb_map_node *head;
        struct logdb_map_node *tail;
        CmpDelegate cmp;
        FreeDelegate key_free;
        FreeDelegate value_free;
    };

    struct logdb_map* logdb_map_new(CmpDelegate cmp, FreeDelegate key_free, FreeDelegate value_free);
    void* logdb_map_get(struct logdb_map* map, void* key);
    struct logdb_map_node* logdb_map_node_get(struct logdb_map* map, void* key);
    int logdb_map_insert(struct logdb_map* map, void* key, void* value);
    int logdb_map_remove(struct logdb_map* map, void* key);
    int logdb_map_destroy(struct logdb_map** p_map);

#ifdef __cplusplus
}
#endif
#endif
