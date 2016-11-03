/*************************************************************************
  > File Name: spx_skp.h
  > Author: shuaixiang
  > Mail: shuaixiang@yuewen.com
  > Created Time: Tue 12 Apr 2016 02:44:04 AM UTC
 ************************************************************************/

#ifndef _SPX_SKP_H_
#define _SPX_SKP_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <unistd.h>
#include "logdb_types.h"
#include "logdb_skp_common.h"

#define SpxSkpMaxLevel 16 
#define SpxSkpNameSize 255

    typedef enum {
        DELETED = -1,
        EXISTED = 0,
        NEWADD = 1
    } stat_t;

    typedef enum{
        SKP_NORMAL = 0,//value list
        SKP_REPLACE = 1//single value, new insert will replace it
    } spx_skp_insert_mode_t;

    struct spx_skp_node_value{
        void * value; 
        struct spx_skp_node_value* next_value;    
        stat_t status;
        int64_t index; //the index of head item
    };

    struct spx_skp_node{
        void * key;
        struct spx_skp_node_value * value;
        int level;
        int size;
        struct spx_skp_node * next_node[0];
    };

    struct spx_skp{
        char name[SpxSkpNameSize];
        struct spx_skp_node * head;
        int level;
        size_t length;
        //SpxSkpType key_type;
        //SpxSkpType value_type;
        CmpDelegate cmp_key;
        CmpDelegate cmp_value;
        FreeDelegate free_key;
        FreeDelegate free_value;
        bool_t is_free_key;
        bool_t is_free_value;
    };


    //-------------interface-------------------------------
    void spx_skp_default_free(void * pfree);
    int spx_skp_level_rand();

    //query
    int spx_skp_query(struct spx_skp *skp, void *key, CopyDelegate copy_value, struct spx_skp_query_result *result);
    int spx_skp_range_query(struct spx_skp *skp, void *start_key, void *end_key, CopyDelegate copy_value, struct spx_skp_query_result *result);
    int spx_skp_bigger_near_query(struct spx_skp *skp, void *key, CopyDelegate copy_value, struct spx_skp_query_result *result);
    int spx_skp_bigger_query(struct spx_skp *skp, void *key, CopyDelegate copy_value, struct spx_skp_query_result *result);
    int spx_skp_smaller_near_query(struct spx_skp *skp, void *key, CopyDelegate copy_value, struct spx_skp_query_result *result);
    int spx_skp_smaller_query(struct spx_skp *skp, void *key, CopyDelegate copy_value, struct spx_skp_query_result *result);

    //query result
    struct spx_skp_query_result *spx_skp_query_result_new();
    int spx_skp_query_result_is_exist(struct spx_skp_query_result *result, void *value, CmpDelegate cmp_value);
    int spx_skp_query_result_destroy(struct spx_skp_query_result *result);
    int spx_skp_query_result_insert(struct spx_skp_query_result *result, void *value);

    //void spx_skp_print(struct spx_skp *sl);
    struct spx_skp_node *spx_skp_find(struct spx_skp *sl, void *key);
    int spx_skp_delete(struct spx_skp *sl, void *key);
    struct spx_skp_node *spx_skp_insert(struct spx_skp * sl, void * key, void * value);
    struct spx_skp_node * spx_skp_insert_replace(struct spx_skp * skp, void * key, void * value);
    struct spx_skp *spx_skp_new(CmpDelegate cmp_key, CmpDelegate cmp_value, const char * skp_name, FreeDelegate free_key, FreeDelegate free_value);
    struct spx_skp *spx_skp_new_tmp(CmpDelegate cmp_key, CmpDelegate cmp_value, const char * skp_name, FreeDelegate free_key, FreeDelegate free_value, bool_t is_free_key);
    int spx_skp_destroy(struct spx_skp * sl);
    int spx_skp_node_remove(struct spx_skp * sl, struct spx_skp_node *q);

#ifdef __cplusplus
}
#endif
#endif

