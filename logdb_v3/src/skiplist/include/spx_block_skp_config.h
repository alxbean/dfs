/*************************************************************************
    > File Name: spx_block_skp_config.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Fri 28 Oct 2016 04:15:33 PM CST
************************************************************************/

#ifndef _SPX_BLOCK_SKP_CONFIG_H_
#define _SPX_BLOCK_SKP_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif
    #include "logdb_queue.h"

    struct spx_block_skp_index{
        char *idx;
        char type;
        struct spx_block_skp_index *next_idx;
    }g_spx_block_skp_idx_head;

    struct spx_block_skp_task{
        void *key;
        void *value;
        spx_block_skp* skp;
    };

    //config
    struct spx_block_skp_index* spx_block_skp_load_index();
    int spx_block_skp_count_config_index();

    //task queue
    int spx_block_skp_task_queue_init();
    struct logdb_queue* spx_block_skp_task_queue_dispatcher(char* key);

    //block_skp pool
    int spx_block_skp_pool_init();
    struct spx_block_skp* spx_block_skp_pool_dispatcher(char* key);

    //task_ctx pool
    int spx_block_skp_task_pool_init();
    struct spx_block_skp_task* spx_block_skp_task_pool_pop();
    int spx_block_skp_task_pool_push(struct spx_block_skp_task* task);

#ifdef __cplusplus
}
#endif
#endif
