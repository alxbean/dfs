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
#include "spx_skp.h"


    //block_skp
    struct spx_block_skp_node{
        void *left_key;
        void *right_key;
        int64_t index;
        int level;
//        int size;//currently, is not necessary
        struct spx_block_skp_node *next_node[0];
    };

    struct spx_block_skp{
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
    };

    //block_skp_list
    struct spx_block_skp_list_node{
        struct spx_block_skp *block_skp;
        struct spx_block_skp_list_node *next_block_skp;
    };

    struct spx_block_skp_list{
        struct spx_block_skp_list_node *head;
        struct spx_block_skp_list_node *tail;
    } g_spx_block_skp_list;

    //skp_queue
    struct spx_skp_queue_node{
        struct spx_skp *skp;
        struct spx_skp_queue_node *next_queue_node;
    };
    
    struct spx_skp_queue{
        struct spx_skp_queue_node *head;
        struct spx_skp_queue_node *tail;
    } g_spx_skp_queue;

    //spx_skp_queue
    struct spx_skp_list *spx_skp_queue_visit();
    int spx_skp_queue_lock();
    int spx_skp_queue_unlock();

    //config op
    struct spx_skp_idx *spx_skp_read_config();
    int spx_skp_read_config_idx_cnt();
    
    //spx_skp_list method 
    struct spx_skp * spx_skp_list_get_push_queue(struct spx_skp_list *skp_list, char *skp_name, err_t *err);
    struct spx_skp * spx_skp_list_get(struct spx_skp_list *skp_list, char *skp_name, err_t *err);
    struct spx_block_skp * spx_block_skp_list_get(char *block_skp_name, spx_skp_type key_type, SpxSkpCmpDelegate cmp_key, spx_skp_type value_type, SpxSkpCmpDelegate cmp_value, SpxSkpFreeDelegate free_key);
    struct spx_skp_list_node *spx_skp_list_add(struct spx_skp_list *skp_list, struct spx_skp *skp);
    int spx_skp_list_delete(struct spx_skp_list *skp_list, char *skp_name);
    struct spx_skp_list *spx_skp_list_new();
    int spx_skp_list_destory(struct spx_skp_list **skp_list_ptr);
    
    //spx_block_skp method
    int spx_block_skp_node_insert(struct spx_block_skp *block_skp, struct spx_block_skp_node *node);
    struct spx_block_skp_node * spx_block_skp_node_new(int level , void * left_key, void *right_key, int64_t index);
    int spx_block_skp_query(struct spx_block_skp *block_skp, void *key, struct spx_skp_query_result *result);

    //create block_skp worker thread
    pthread_t spx_block_skp_thread_new(SpxLogDelegate *log, err_t *err);

#ifdef __cplusplus
}
#endif
#endif

