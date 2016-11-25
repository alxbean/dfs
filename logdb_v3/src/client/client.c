/*************************************************************************
    > File Name: client.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Thu 24 Nov 2016 04:05:10 PM CST
************************************************************************/
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "msgpk_build_tree.h"
#include "msgpk_tree.h"

#include "messagepack.h"
#include "skiplist.h"
#include "spx_log.h"
#include "thread_pool.h"
#include "logdb_debug.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int total = 0;
void insert_test(void* q){
    int count = *(int*)q;
    const char* request = {"111111111"};
    int i;
    for (i = 0; i < count; i++){
        int age = rand()%120;
        struct tree_context* ctx = msgpk_build_init();
        msgpk_build_map_string_int(ctx, "age", 3, age);
        
        //msgpk_tree_print_json(ctx->root);
        //printf("\n");
        //struct msgpk_object* obj = msgpk_tree_find_rule_node(ctx->root, 1, OBJ_TYPE_STR, "age"); 
        //printf("age:%d\n", obj->value.int32_val);
        
        pthread_mutex_lock(&mutex);
        printf("thread:%ld ======================================>%d\n", pthread_self(), total++);
        char* unid = msgpk_tree_add(ctx->root, strlen(request), (char*)request);
        pthread_mutex_unlock(&mutex);
        free(unid);
        msgpk_build_free(ctx);
    }
}

int main(){
    //init
    SpxLogDelegate* log = spx_log;
    //    printf("%d.\n",getpid());
    err_t err = 0;
    pthread_t block_tid = spx_block_skp_thread_new(log, &err);
    if (err != 0){
        SpxLog2(log, SpxLogError, err, "spx_skp_serial_block_skp_thread_new failed");
        exit(err);
    }

    spx_block_skp_config_task_queue_init();
    spx_block_skp_config_pool_init();
    spx_block_skp_config_task_pool_init();
    //thread_pool_init(spx_block_skp_count_config_index());
    thread_pool_init(20);

    //------------------------------------------------
    clock_t start;
    clock_t end;
    pthread_t tid[10];

    int count = 10000000;
    int i;
    srand((int) time(0));

    start = clock();
    for(i = 0; i < 10; i++){
        if (pthread_create(&tid[i], NULL, (void*)insert_test, &count) != 0){
            logdb_debug("pthread_create failed\n");
        }
        printf("------------------------------------------------->thread:%d start to  work\n", i);
        pthread_join(tid[i], NULL);
    }
    end = clock();

    double cost = (double)(end - start)/CLOCKS_PER_SEC;
    //pthread_join(block_tid, NULL);
    printf("======================================\n");
    printf("add %d cost time: %dms \n", count, (int)(cost * 1000));
    return 0;
}

