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

    struct spx_block_skp_index* spx_block_skp_load_index();
    int spx_block_skp_count_config_index();

#ifdef __cplusplus
}
#endif
#endif
