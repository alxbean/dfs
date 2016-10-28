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
    #include "logdb_map.h"

    struct spx_block_skp_config{
        char *idx;
        char type;
        struct spx_skp_idx *next_idx;
    };

    struct logdb_map* spx_block_skp_config_init();
    int spx_block_skp_config_idx_cnt();

#ifdef __cplusplus
}
#endif
#endif
