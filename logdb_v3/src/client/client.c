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

int main(){
    //init
    SpxLogDelegate* log = spx_log;
    //    printf("%d.\n",getpid());
    err_t err = 0;
    pthread_t tid = spx_block_skp_thread_new(log, &err);
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

    int count = 10;
    int i;
    const char* request = {"11111111111111111111111111111111111111111111111111111111111111111111111"};
    srand((int) time(0));

    start = clock();
    for (i = 0; i < count; i++){
        int age = rand()%120;
        struct tree_context* ctx = msgpk_build_init();
        msgpk_build_map_string_int(ctx, "age", 3, age);
        //msgpk_tree_print_json(ctx->root);
        //printf("\n");
        //struct msgpk_object* obj = msgpk_tree_find_rule_node(ctx->root, 1, OBJ_TYPE_STR, "age"); 
        //printf("age:%d\n", obj->value.int32_val);
        msgpk_tree_add(ctx->root, strlen(request), (char*)request);
        msgpk_build_free(ctx);
    }
    end = clock();

    double cost = (double)(end - start)/CLOCKS_PER_SEC;
    printf("======================================\n");
    printf("add %d cost time: %dms \n", count, (int)(cost * 1000));

    return 0;
}

