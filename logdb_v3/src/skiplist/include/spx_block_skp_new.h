/*************************************************************************
  > File Name: spx_block_skp.h
  > Author: shuaixiang
  > Mail: shuaixiang@yuewen.com
  > Created Time: Tue 12 Apr 2016 02:44:04 AM UTC
 ************************************************************************/

#ifndef _SPX_BLOCK_SKP_NEW_H_
#define _SPX_BLOCK_SKP_NEW_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <unistd.h>
#include <pthread.h>
#include "spx_types.h"
#include "spx_skp.h"

    //block_skp
    struct spx_block_skp_node{
        void *left_key;
        void *right_key;
        int64_t index;
        int level;
        pthread_mutex_t mutex;//mutex for spx_block_skp_node
        struct spx_block_skp_node *next_node[0];
    };

    struct spx_block_skp {
        char name[SpxSkpNameSize];
        struct spx_block_skp_node *head;
        int level;
        size_t length;
        spx_skp_type key_type;
        spx_skp_type value_type;
        SpxSkpCmpDelegate cmp_key;
        SpxSkpCmpDelegate cmp_value;
        SpxSkpFreeDelegate free_key;
        pthread_mutex_t mutex;//mutex for spx_block_skp
    }

#ifdef __cplusplus
}
#endif
#endif

