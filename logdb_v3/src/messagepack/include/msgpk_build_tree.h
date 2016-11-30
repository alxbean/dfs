/*************************************************************************
> File Name: msgpk_build_tree.h
> Author: shuaixiang
> Mail: shuaixiang@yuewen.com
> Created Time: Tue 05 Apr 2016 06:21:32 AM UTC
************************************************************************/

#ifndef _MSGPK_BUILD_TREE_H_
#define _MSGPK_BUILD_TREE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "msgpk_packer.h"
#include "msgpk_define.h"
    //data struct
    struct tree_context{
        struct msgpk_object *root;
        struct msgpk_object *node;
        bool_t is_child;
    };

    //interface
    struct tree_context * msgpk_build_init();
    void msgpk_build_free(struct tree_context * ctx);

    //PackTree
    void msgpk_build_tree_string(struct tree_context *ctx, char* str, size_t len);
    void msgpk_build_tree_char(struct tree_context *ctx, char d);
    void msgpk_build_tree_short(struct tree_context *ctx, short d);
    void msgpk_build_tree_int(struct tree_context *ctx, int d);
    void msgpk_build_tree_long(struct tree_context *ctx, long d);
    void msgpk_build_tree_longlong(struct tree_context *ctx, long long d);
    void msgpk_build_tree_unsigned_char(struct tree_context *ctx, unsigned char d);
    void msgpk_build_tree_unsigned_short(struct tree_context *ctx, unsigned short d);
    void msgpk_build_tree_unsigned_int(struct tree_context *ctx, unsigned int d);
    void msgpk_build_tree_unsigned_long(struct tree_context *ctx, unsigned long d);
    void msgpk_build_tree_unsigned_longlong(struct tree_context *ctx, unsigned long long d);
    void msgpk_build_tree_nil(struct tree_context *ctx);
    void msgpk_build_tree_true(struct tree_context *ctx);
    void msgpk_build_tree_false(struct tree_context *ctx);
    void msgpk_build_tree_float(struct tree_context *ctx, float d);
    void msgpk_build_tree_double(struct tree_context *ctx, double d);
    void msgpk_build_tree_bin(struct tree_context *ctx, ubyte_t * b, size_t len);
    void msgpk_build_tree_array_begin(struct tree_context *ctx);
    void msgpk_build_tree_array_end(struct tree_context *ctx, int len);
    void msgpk_build_tree_map_begin(struct tree_context *ctx);
    void msgpk_build_tree_map_end(struct tree_context *ctx, int len);

    //PackMap
    void msgpk_build_map_string_int(struct tree_context *ctx, char* k, size_t len,  int v);
    void msgpk_build_map_string_long(struct tree_context *ctx, char* k, size_t len,  long v);
    void msgpk_build_map_string_longlong(struct tree_context *ctx, char* k, size_t len,  long long v);
    void msgpk_build_map_string_float(struct tree_context *ctx, char* k, size_t len,  float v);
    void msgpk_build_map_string_double(struct tree_context *ctx, char* k, size_t len, double v);
    void msgpk_build_map_string_string(struct tree_context *ctx, char* k, size_t len,  char* v, size_t vlen);
    void msgpk_build_map_string_bin(struct tree_context *ctx, char* k, size_t len, ubyte_t *v, size_t vlen);
    void msgpk_build_map_int_int(struct tree_context *ctx, int k, int v);
    void msgpk_build_map_int_long(struct tree_context *ctx, int k, long v);
    void msgpk_build_map_int_longlong(struct tree_context *ctx, int k, long long v);
    void msgpk_build_map_int_float(struct tree_context *ctx, int k, float v);
    void msgpk_build_map_int_double(struct tree_context *ctx, int k, double v);
    void msgpk_build_map_int_string(struct tree_context *ctx, int k, char* v, size_t vlen);
    void msgpk_build_map_int_bin(struct tree_context *ctx, int k, ubyte_t *v, size_t vlen);

    //Query Tree
    struct msgpk_object* msgpk_tree_find_rule_node(struct msgpk_object *root , int argc, ...);
    void msgpk_tree_print(struct msgpk_object *obj, int space);
    void msgpk_tree_print_json(struct msgpk_object *obj);
    //free tree
    void msgpk_tree_free(struct msgpk_object *obj);


#ifdef __cplusplus
}
#endif
#endif

