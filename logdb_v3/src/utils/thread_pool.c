/*************************************************************************
    > File Name: thread_pool.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Tue 27 Sep 2016 10:16:06 AM CST
 ************************************************************************/
 #include <assert.h>
 #include "thread_pool.h"

typedef struct task{
    void (*process) (void *arg);
    void *arg;
    struct task *next;
} thread_task_t;

typedef struct {
     pthread_mutex_t task_queue_lock;
     pthread_cond_t task_queue_ready;

     thread_task_t *task_queue_head;

     bool_t shutdown;
     pthread_t *worker_id;

     int max_thread_num;
     int cur_queue_size;
} thread_pool_t;

static thread_pool_t *g_thread_pool = NULL;

static void *thread_routine(void *arg);

void thread_pool_init(int max_thread_num){/*{{{*/
    g_thread_pool = (thread_pool_t *) malloc(sizeof(*g_thread_pool));

    pthread_mutex_init(&(g_thread_pool->task_queue_lock), NULL);
    pthread_cond_init(&(g_thread_pool->task_queue_ready), NULL);

    g_thread_pool->task_queue_head = NULL;

    g_thread_pool->max_thread_num = max_thread_num;
    g_thread_pool->cur_queue_size = 0;

    g_thread_pool->shutdown = false;
    g_thread_pool->worker_id = (pthread_t *) malloc(max_thread_num *sizeof(pthread_t));
    int i = 0;
    for (i = 0; i < max_thread_num; i++){
        pthread_create(&(g_thread_pool->worker_id[i]), NULL, thread_routine, NULL);
    }
}/*}}}*/

int pool_task_add(void (*process) (void *arg), void *arg){/*{{{*/
    if (NULL == process){
        printf("process is NULL\n");
        return -1;
    }

    thread_task_t *new_task = (thread_task_t *) malloc(sizeof(*new_task));
    new_task->process = process;
    new_task->arg = arg;
    new_task->next = NULL;

    pthread_mutex_lock(&(g_thread_pool->task_queue_lock));
    thread_task_t *task = g_thread_pool->task_queue_head;
    if (task != NULL){
        while (task->next != NULL)
            task = task->next;
        task->next = new_task;
    } else {
        g_thread_pool->task_queue_head = new_task;
    }

    assert(g_thread_pool->task_queue_head != NULL);
    g_thread_pool->cur_queue_size++;
    pthread_mutex_unlock(&(g_thread_pool->task_queue_lock));

    pthread_cond_signal(&(g_thread_pool->task_queue_ready));

    return 0;
}
/*}}}*/

int thread_pool_destroy(){/*{{{*/
    if (true == g_thread_pool->shutdown)
        return -1;
    g_thread_pool->shutdown = true;

    pthread_cond_broadcast(&(g_thread_pool->task_queue_ready));

    int i;
    for (i = 0; i < g_thread_pool->max_thread_num; i++){
        pthread_join(g_thread_pool->worker_id[i], NULL);
    }

    free(g_thread_pool->worker_id);

    pthread_mutex_destroy(&(g_thread_pool->task_queue_lock));
    pthread_cond_destroy(&(g_thread_pool->task_queue_ready));

    free(g_thread_pool);
    g_thread_pool = NULL;

    return 0;
}/*}}}*/

static void *thread_routine(void *arg){/*{{{*/
    logdb_debug("starting thread 0x%x\n", (unsigned int )pthread_self());
    while (1){
        pthread_mutex_lock(&(g_thread_pool->task_queue_lock));

        while (0 == g_thread_pool->cur_queue_size && false == g_thread_pool->shutdown){
            logdb_debug("thread 0x%x is waitting\n", (unsigned int)pthread_self());
            pthread_cond_wait(&(g_thread_pool->task_queue_ready), &(g_thread_pool->task_queue_lock));
        }

        if (true == g_thread_pool->shutdown){
            pthread_mutex_unlock(&(g_thread_pool->task_queue_lock));
            logdb_debug("thread 0x%x will exit\n", (unsigned int)pthread_self());
            pthread_exit(NULL);
        }

        logdb_debug("thread 0x%x is starting to work\n", (unsigned int )pthread_self());

        assert(g_thread_pool->cur_queue_size != 0);
        assert(g_thread_pool->task_queue_head != NULL);

        g_thread_pool->cur_queue_size--;
        thread_task_t *task = g_thread_pool->task_queue_head;
        g_thread_pool->task_queue_head = task->next;

        pthread_mutex_unlock(&(g_thread_pool->task_queue_lock));

        (*(task->process))(task->arg);
        free(task);
        task = NULL;
        logdb_debug("thread task size:%d\n", g_thread_pool->cur_queue_size);
    }
}/*}}}*/

void test(void *arg){
    printf("worker_id is 0x%x, working on task %d\n", (unsigned int )pthread_self(), *(int *)arg);
    sleep(1);
}

//int main(int argc, char **argv){
//    thread_pool_init(3);    
//    int *workingnum = (int *) malloc(sizeof(int)*10);
//
//    int i;
//    for (i = 0; i < 10; i++){
//        workingnum[i] = i;
//        pool_task_add(test, &workingnum[i]);
//    }
//
//    sleep(5);
//
//    thread_pool_destroy();
//    free(workingnum);
//
//    return 0;
//}


