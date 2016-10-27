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

    typedef int (*CmpDelegate)(const void *, const void *);

    struct logdb_map_node {
        void* key;
        void* value;
        struct logdb_map_node* next_node;
    };

    struct logdb_map {
        size_t size;
        struct logdb_queue_node *head;
        struct logdb_queue_node *tail;
        CmpDelegate cmp;
    };

    struct logdb_map* logdb_map_new();
    void* logdb_map_get(struct logdb_map* map, void* key);

#ifdef __cplusplus
}
#endif
#endif
