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
#include "spx_types.h"

    //typedef enum{
    //    SKP_TYPE_INT = 0,
    //    SKP_TYPE_STR = 1,
    //    SKP_TYPE_LONG = 2,
    //    SKP_TYPE_MD = 3
    //} SKP_TYPE; 
    typedef int (*SpxSkpCmpDelegate)(const void *, const void *);
    typedef void (*SpxSkpFreeDelegate)(void *key);

    typedef enum{
        DELETED = -1,
        EXISTED = 0,
        NEWADD = 1
    } stat_t;

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
        //char key_type;
        //char value_type;
        //the array of nextNode is the column of the node
        struct spx_skp_node * next_node[0];
    };

    struct spx_skp{
        char * name;
        struct spx_skp_node * head;
        int level;
        size_t length;
        SpxSkpCmpDelegate cmp_key;
        SpxSkpCmpDelegate cmp_value;
        SpxSkpFreeDelegate free_key;
        SpxSkpFreeDelegate free_value;
        bool_t is_free_kv;
    };

    //for saving query result of spx_skp_node_value
    struct spx_skp_value_list_node{
        struct spx_skp_node_value *value;
        struct spx_skp_value_list_node *next_value_list_node;
    };

    struct spx_skp_value_list{
        struct spx_skp_value_list_node *head;
        struct spx_skp_value_list_node *tail;
    };

    struct spx_skp_list_node{
        struct spx_skp *skp;
        struct spx_skp_list_node  *next_skp;
    };

    struct spx_skp_list{
        struct spx_skp_list_node *head; 
        struct spx_skp_list_node *tail;
    } spx_skp_list_head;


    //-------------Interface-------------------------------
    //query
    struct spx_skp_value_list* spx_skp_query(struct spx_skp *sl, void *key);
    struct spx_skp_value_list* spx_skp_range_query(struct spx_skp *sl, void *start_key, void *end_key);
    struct spx_skp_value_list* spx_skp_bigger_near_query(struct spx_skp *sl, void *key);
    struct spx_skp_value_list* spx_skp_bigger_query(struct spx_skp *sl, void *key);
    struct spx_skp_value_list* spx_skp_smaller_near_query(struct spx_skp *sl, void *key);
    struct spx_skp_value_list* spx_skp_smaller_query(struct spx_skp *sl, void *key);

    //spx_skp operation
    //void spx_skp_print(struct spx_skp *sl);
    struct spx_skp_node* spx_skp_find(struct spx_skp *sl, void *key);
    int spx_skp_delete(struct spx_skp *sl, void *key);
    struct spx_skp_node * spx_skp_insert(struct spx_skp * sl, void * key, void * value);
    struct spx_skp * spx_skp_new(SpxSkpCmpDelegate cmp_key, SpxSkpCmpDelegate cmp_value, const char * sl_name, SpxSkpFreeDelegate free_key, SpxSkpFreeDelegate free_value);
    struct spx_skp * spx_skp_new_tmp(SpxSkpCmpDelegate cmp_key, SpxSkpCmpDelegate cmp_value, const char * skp_name, SpxSkpFreeDelegate free_key, SpxSkpFreeDelegate free_value);
    int spx_skp_destory(struct spx_skp * sl);
    void spx_skp_value_list_free(struct spx_skp_value_list *value_list);
    int spx_skp_node_remove(struct spx_skp * sl, struct spx_skp_node *q);


    //cmp method
    int cmp_md(const void *a, const void *b);
    int cmp_int(const void *a, const void *b);
    int cmp_long(const void *a, const void *b);
    int cmp_float(const void *a, const void *b);
    int cmp_double(const void *a, const void *b);
    int cmp_str(const void *a, const void *b);
    
#ifdef __cplusplus
}
#endif
#endif

