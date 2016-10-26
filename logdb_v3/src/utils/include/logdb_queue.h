/*************************************************************************
    > File Name: logdb_queue.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Thu 20 Oct 2016 10:36:55 AM CST
 ************************************************************************/

#ifndef _LOGDB_QUEUE_H_
#define _LOGDB_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif
    #include <stdio.h>
    #include <stdlib.h>
    #include <pthread.h>

    struct logdb_queue_node {
        void* value;
        struct logdb_queue_node* next_node;
    };

    struct logdb_queue {
        char name[100];
        size_t size;
        pthread_mutex_t mutex;
        struct logdb_queue_node *head;
        struct logdb_queue_node *tail;
    };

    struct logdb_queue* logdb_queue_new(char* name);
    int logdb_queue_destroy(struct logdb_queue** p_queue);
    int logdb_queue_push(struct logdb_queue* queue, void* value);
    void* logdb_queue_pop(struct logdb_queue* queue);
    int logdb_queue_is_empty(struct logdb_queue* queue);

#ifdef __cplusplus
}
#endif
#endif
