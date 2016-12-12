/*************************************************************************
    > File Name: logdb_queue.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Wed 26 Oct 2016 04:55:31 PM CST
 ************************************************************************/
#include <string.h>
#include "logdb_queue.h"

static struct logdb_queue_node* logdb_queue_node_new(void* value);
static int logdb_queue_node_destroy(struct logdb_queue_node** p_node);

static struct logdb_queue_node* logdb_queue_node_new(void* value){/*{{{*/
    struct logdb_queue_node* new_node = (struct logdb_queue_node *) malloc(sizeof(*new_node));
    if (NULL == new_node){
        printf("malloc new_node failed in logdb_queue_node_new\n");
        return NULL;
    }

    new_node->value = value;
    new_node->next_node = NULL;

    return new_node;
}/*}}}*/

static int logdb_queue_node_destroy(struct logdb_queue_node** p_node){/*{{{*/
    if (NULL == p_node){
        printf("p_node is NULL in logdb_queue_node_destroy\n");
        return -1;
    }

    struct logdb_queue_node* node = *p_node;
    if (NULL == node){
        printf("node is NULL in logdb_queue_node_destroy\n");
        return -1;
    }

    node->value = NULL;
    node->next_node = NULL;
    free(node);
    *p_node = NULL;

    return 0;
}/*}}}*/

struct logdb_queue* logdb_queue_new(char* name){/*{{{*/
    struct logdb_queue* new_queue = (struct logdb_queue*) malloc(sizeof(*new_queue));
    if (NULL == new_queue){
        printf("malloc new_queue failed in logdb_queue_new\n");
        return NULL;
    }

    strncpy(new_queue->name, name, strlen(name));
    pthread_mutex_init(&new_queue->mutex, NULL); 
    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->size = 0;

    return new_queue;
}/*}}}*/

int logdb_queue_destroy(struct logdb_queue** p_queue){/*{{{*/
    if (NULL == p_queue){
        printf("p_queue is NULL in logdb_queue_destory\n");
        return -1;
    }

    struct logdb_queue* queue = *p_queue;
    if (NULL == queue){
        printf("queue is NULL in logdb_queue_destory\n");
        return -1;
    }

    while (queue->head != NULL)
        logdb_queue_pop(queue);

    free(queue);
    *p_queue = NULL;

    return 0;
}/*}}}*/

int logdb_queue_is_empty(struct logdb_queue* queue){/*{{{*/
    if (NULL == queue){
        printf("queue is NULL in logdb_queue_is_empty\n");
        return -1;
    }

    if (0 == queue->size)
        return 1;
    else
        return 0;
}/*}}}*/

int logdb_queue_push(struct logdb_queue* queue, void* value){/*{{{*/
    if (NULL == queue){
        printf("queue is NULL in logdb_queue_push\n");
        return -1;
    }

    if (NULL == value){
        printf("value is NULL in logdb_queue_push\n");
        return -1;
    }

    struct logdb_queue_node* new_node = logdb_queue_node_new(value);
    if (NULL == new_node){
        printf("logdb_queue_node_new failed\n");
        return -1;
    }

    pthread_mutex_lock(&queue->mutex);
    if (NULL == queue->head){
        queue->head = new_node;
        queue->tail = new_node;
        queue->size++;
    } else {
        queue->tail->next_node = new_node;
        queue->tail = new_node;
        queue->size++;
    }
    pthread_mutex_unlock(&queue->mutex);

    return 0;
}/*}}}*/

void* logdb_queue_pop(struct logdb_queue* queue){/*{{{*/
    if (NULL == queue){
        printf("queue is NULL in logdb_queue_pop\n");
        return NULL;
    }

    void* value = NULL;

    if (0 == queue->size){
        //printf("queue:%s is empty\n", queue->name);
    } else {
        pthread_mutex_lock(&queue->mutex);
        struct logdb_queue_node *dead_node = queue->head;
        queue->head = queue->head->next_node;
        if (NULL == queue->head)
            queue->tail = NULL;
        queue->size--;
        pthread_mutex_unlock(&queue->mutex);
        value = dead_node->value;
        logdb_queue_node_destroy(&dead_node);
    }

    return value;
}/*}}}*/

