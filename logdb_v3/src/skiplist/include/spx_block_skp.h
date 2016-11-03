/*************************************************************************
  > File Name: spx_block_skp.h
  > Author: shuaixiang
  > Mail: shuaixiang@yuewen.com
  > Created Time: Tue 12 Apr 2016 02:44:04 AM UTC
 ************************************************************************/

#ifndef _SPX_BLOCK_SKP_H_
#define _SPX_BLOCK_SKP_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <unistd.h>
#include <pthread.h>
#include "spx_types.h"
#include "spx_block_skp_common.h"
#include "logdb_skp.h"

    //block_skp
    struct spx_block_skp_node{
        void *left_key;
        void *right_key;
        int64_t index;
        int level;
        pthread_mutex_t mutex;
        struct spx_block_skp_node *next_node[0];
    };

    struct spx_block_skp {
        char name[100];
        struct spx_block_skp_node *head;
        int level;
        size_t length;
        SpxSkpType key_type;
        SpxSkpType value_type;
        CmpDelegate cmp_key;
        CmpDelegate cmp_value;
        FreeDelegate free_key;
        FreeDelegate free_value;
        pthread_mutex_t mutex;//mutex for spx_block_skp
    };

    //spx_block_skp method
    struct spx_block_skp * spx_block_skp_new(SpxSkpType key_type, SpxSkpType value_type, CmpDelegate cmp_key, CmpDelegate cmp_value, const char * block_skp_name, FreeDelegate free_key, FreeDelegate free_value);
    int spx_block_skp_destroy(struct spx_block_skp ** p_block_skp);
    int spx_block_skp_query(struct spx_block_skp *block_skp, void *key, struct spx_skp_query_result *result);
    struct spx_block_skp_node * spx_block_skp_node_new(int level , void * left_key, void *right_key, int64_t index);
    int spx_block_skp_node_insert(struct spx_block_skp *block_skp, struct spx_block_skp_node *node);

    //create block_skp worker thread
    pthread_t spx_block_skp_thread_new(SpxLogDelegate *log, err_t *err);
    
#ifdef __cplusplus
}
#endif
#endif

