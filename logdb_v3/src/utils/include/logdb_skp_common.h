/*************************************************************************
    > File Name: logdb_skp_common.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Thu 20 Oct 2016 10:36:55 AM CST
 ************************************************************************/

#ifndef _LOGDB_SKP_COMMON_H_
#define _LOGDB_SKP_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif
    typedef enum{
        SKP_INT = 0,
        SKP_STR = 1,
        SKP_LONG = 2,
        SKP_MD = 3,
        SKP_FLOAT = 4,
        SKP_DOUBLE = 5
    } SpxSkpType; 

    //cmp method
    int cmp_int(const void *a, const void *b);
    int cmp_long(const void *a, const void *b);
    int cmp_float(const void *a, const void *b);
    int cmp_double(const void *a, const void *b);
    int cmp_str(const void *a, const void *b);

    //for saving query result set 
    struct spx_skp_query_result_node{
        void *value;
        struct spx_skp_query_result_node *next_result_node;
    };

    struct spx_skp_query_result{
        struct spx_skp_query_result_node *head;
        struct spx_skp_query_result_node *tail;
    };
#ifdef __cplusplus
}
#endif
#endif
