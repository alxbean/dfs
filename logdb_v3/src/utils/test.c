/*************************************************************************
    > File Name: test.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Thu 20 Oct 2016 05:29:20 PM CST
 ************************************************************************/

#include "logdb_queue.h"
#include "time.h"
#include <unistd.h>

int count = 100000;
void *thread_push(void *q){
    struct logdb_queue *queue = (struct logdb_queue *) q;
    int i;
    for (i = 0; i < count; i++){
        int *x = (int *)malloc(sizeof(int));
        *x = i;
        logdb_queue_push(queue, x);
        printf("pid:%u push:%d queue size:%zd\n", (unsigned int)pthread_self(), *x, queue->size);
        sleep(1);
    }

    return NULL;
}

void *thread_pop(void *q){
    struct logdb_queue *queue = (struct logdb_queue *) q;
    while (!logdb_queue_is_empty(queue)){
        int *x = (int *)logdb_queue_pop(queue);
        if (x != NULL){
            printf("pid:%u pop:%d queue size:%zd\n", (unsigned int)pthread_self(), *x, queue->size);
            free(x);
        }
        sleep(1);
    }

    return NULL;
}


 int main(){
     struct logdb_queue *test_queue = logdb_queue_new("test_queue");
     clock_t start ,finish;
     pthread_t push_tid[10], pop_tid[10];
     int i;

     start = clock();
     for (i = 0; i < 12; i++){
        pthread_create(&push_tid[i], NULL, (void *) thread_push, test_queue); 
     }

     for (i = 0; i < 11; i++){
        pthread_create(&pop_tid[i], NULL, (void *) thread_pop, test_queue); 
     }
        sleep(1);

     for (i = 0; i< 12; i++){
        pthread_join(push_tid[i], NULL);
     }

     for (i = 0; i< 11; i++){
        pthread_join(pop_tid[i], NULL);
     }
     finish = clock();
     
     printf("push and pop: %.0fms for count:%d\n", (double)(((finish - start)*1000)/CLOCKS_PER_SEC), count);
     logdb_queue_destroy(&test_queue);
 }

