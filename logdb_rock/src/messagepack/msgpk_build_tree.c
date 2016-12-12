/*************************************************************************
    > File Name: msgpk_build_tree.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Tue 05 Apr 2016 06:56:02 AM UTC
 ************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include "msgpk_build_tree.h"
#include "spx_string.h"

#define MsgpkBuildTreeStackSize 100

static struct msgpk_object * g_build_stack[MsgpkBuildTreeStackSize];
static int top = 0;//Mutex
/*
 *declare
 */
static void msgpk_build_push_stack(struct msgpk_object *obj);
static struct msgpk_object * msgpk_build_pop_stack();
static struct msgpk_object * msgpk_build_node_string(char* str, size_t len, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_char(char d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_short(short d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_int(int d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_long(long d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_longlong(long long d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_unsigned_char(unsigned char d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_unsigned_short(unsigned short d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_unsigned_int(unsigned int d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_unsigned_long(unsigned long d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_unsigned_longlong(unsigned long long d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_nil(struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_true(struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_false(struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_float(float d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_double(double d, struct msgpk_object *obj, bool_t isKey);
static struct msgpk_object * msgpk_build_node_bin(ubyte_t * b, size_t len, struct msgpk_object *obj, bool_t isKey);

static struct msgpk_object * msgpk_tree_hit_node(struct msgpk_object *obj, object_type keyType, object_value keyValue);
static void msgpk_tree_print_blank(int n);

static struct msgpk_object * msgpk_tree_hit_node(struct msgpk_object *obj, object_type keyType, object_value keyValue){/*{{{*/
    if(obj->isKey == true){
        switch(keyType){
            case OBJ_TYPE_STR:
                //printf("\"%s\" : \"%s\"\n", keyValue.str_val, obj->key.str_val);
                if(strncmp(obj->key.str_val, keyValue.str_val, obj->key_len) == 0)
                    return obj;
                break;
            case OBJ_TYPE_INT8:
                //printf("(int8): %d : %d\n", keyValue.int8_val, obj->key.int8_val);
                if(keyValue.int8_val == obj->key.int8_val)
                    return obj;
                break;
            case OBJ_TYPE_INT16:
                //printf("(int16): %d : %d\n", keyValue.int16_val, obj->key.int16_val);
                if(keyValue.int16_val == obj->key.int16_val)
                    return obj;
                break;
            case OBJ_TYPE_INT32:
                //printf("(int32): %d : %d\n", keyValue.int32_val, obj->key.int32_val);
                if(keyValue.int16_val == obj->key.int16_val)
                    return obj;
                break;
            case OBJ_TYPE_INT64:
                //printf("(int64): %ld : %ld\n", keyValue.int64_val, obj->key.int64_val);
                if(keyValue.int64_val == obj->key.int64_val)
                    return obj;
                break;
            case OBJ_TYPE_UINT8:
                //printf("(uint8): %u : %u\n", keyValue.uint8_val, obj->key.uint8_val);
                if(keyValue.uint8_val == obj->key.uint8_val)
                    return obj;
                break;
            case OBJ_TYPE_UINT16:
                //printf("(uint16): %u : %u\n", keyValue.uint16_val, obj->key.uint16_val);
                if(keyValue.uint16_val == obj->key.uint16_val)
                    return obj;
                break;
            case OBJ_TYPE_UINT32:
                //printf("(uint32): %u : %u\n", keyValue.uint32_val, obj->key.uint32_val);
                if(keyValue.uint32_val == obj->key.uint32_val)
                    return obj;
                break;
            case OBJ_TYPE_UINT64:
                //printf("(uint64): %lu : %lu\n", keyValue.uint64_val, obj->key.uint64_val);
                if(keyValue.uint64_val == obj->key.uint64_val)
                    return obj;
                break;
            case OBJ_TYPE_POSITIVE_INT:
                //printf("(fixInt): %u : %u\n", keyValue.uint8_val, obj->key.uint8_val);
                if(keyValue.uint8_val == obj->key.uint8_val)
                    return obj;
                break;
            case OBJ_TYPE_NEGATIVE_INT:
                //printf("(fixInt): %d : %d\n", keyValue.int8_val, obj->key.int8_val);
                if(keyValue.int8_val == obj->key.int8_val)
                    return obj;
                break;
            default:
                printf("not found");
                return NULL;
        }
    }

    if(obj->next != NULL)
       return msgpk_tree_hit_node(obj->next, keyType, keyValue); 
    if(obj->child != NULL)
       return msgpk_tree_hit_node(obj->child, keyType, keyValue); 

    return NULL;
}/*}}}*/

static void msgpk_tree_print_blank(int n){/*{{{*/
    int i = 0;
    for(i = 0; i < n; i++){
        printf(" ");
    }
}/*}}}*/

/*
 *define
 */
struct tree_context * msgpk_build_init(){/*{{{*/
        top = 0;
        struct tree_context *ctx = (struct tree_context *) calloc(1, sizeof(struct tree_context));
        struct msgpk_object *root = new_msgpk_object();
        if(NULL == root){
            return NULL;
        }

        ctx->root = root;
        ctx->node = root;
        ctx->is_child = true;
        return ctx;
}/*}}}*/

void msgpk_build_free(struct tree_context * ctx){/*{{{*/
    msgpk_tree_free(ctx->root);
    free(ctx);
}/*}}}*/

static void msgpk_build_push_stack(struct msgpk_object *obj){/*{{{*/
    g_build_stack[top++] = obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_pop_stack(){/*{{{*/
    if( top > 0){
        top--;
        return g_build_stack[top];
    } else {
        return g_build_stack[top];
    }
}/*}}}*/

//PackNode
static struct msgpk_object * msgpk_build_node_string(char* str, size_t len, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }
    
    if (false == isKey){
        obj->obj_type = OBJ_TYPE_STR; 
        obj->value.str_val = (char*) calloc(1, sizeof(char)*(len + 1)); // + 1 for '\0'
        obj->obj_len = len;
        memcpy(obj->value.str_val, str, len);
        *(obj->value.str_val + len) = '\0';
    } else {
        obj->isKey = isKey;
        obj->key_type = OBJ_TYPE_STR;
        obj->key_len = len;
        obj->key.str_val = (char*) calloc(1, sizeof(char)*(len + 1));
        memcpy(obj->key.str_val, str, len);
        *(obj->key.str_val + len) = '\0';
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_char(char d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj ){
        return NULL;
    }

    if (false == isKey){
        obj->obj_type = OBJ_TYPE_INT8;
        obj->value.int8_val = d;
    } else {
        obj->isKey = isKey;
        obj->key_type = OBJ_TYPE_INT8;
        obj->key.int8_val = d;
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_short(short d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj ){
        return NULL;
    }

    if (false == isKey){
        if ( 2 == sizeof(short) ){
            obj->obj_type = OBJ_TYPE_INT16;
            obj->value.int16_val = d;
        } else if ( 4 == sizeof(short) ){
            obj->obj_type = OBJ_TYPE_INT32;
            obj->value.int32_val = d;
        } else{
            obj->obj_type = OBJ_TYPE_INT64;
            obj->value.int64_val = d;
        }
    } else {
        obj->isKey = isKey;
        if ( 2 == sizeof(short) ){
            obj->key_type = OBJ_TYPE_INT16;
            obj->key.int16_val = d;
        } else if ( 4 == sizeof(short) ){
            obj->key_type = OBJ_TYPE_INT32;
            obj->key.int32_val = d;
        } else{
            obj->key_type = OBJ_TYPE_INT64;
            obj->key.int64_val = d;
        }
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_int(int d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        if ( 2 == sizeof(int) ){
            obj->obj_type = OBJ_TYPE_INT16;
            obj->value.int16_val = d;
        } else if( 4 == sizeof(int) ){
            obj->obj_type = OBJ_TYPE_INT32;
            obj->value.int32_val = d;
        } else{
            obj->obj_type = OBJ_TYPE_INT64;
            obj->value.int32_val = d;
        }
    } else {
        obj->isKey = isKey;
        if ( 2 == sizeof(int) ){
            obj->key_type = OBJ_TYPE_INT16;
            obj->key.int16_val = d;
        } else if( 4 == sizeof(int) ){
            obj->key_type = OBJ_TYPE_INT32;
            obj->key.int32_val = d;
        } else{
            obj->key_type = OBJ_TYPE_INT64;
            obj->key.int32_val = d;
        }
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_long(long d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        if ( 2 == sizeof(long) ){
            obj->obj_type = OBJ_TYPE_INT16;
            obj->value.int16_val = d;
        } else if ( 4 == sizeof(long) ){
            obj->obj_type = OBJ_TYPE_INT32;
            obj->value.int32_val = d;
        } else {
            obj->obj_type = OBJ_TYPE_INT64;
            obj->value.int64_val = d;
        }
    } else {
        obj->isKey = isKey;
        if ( 2 == sizeof(long) ){
            obj->key_type = OBJ_TYPE_INT16;
            obj->key.int16_val = d;
        } else if ( 4 == sizeof(long) ){
            obj->key_type = OBJ_TYPE_INT32;
            obj->key.int32_val = d;
        } else {
            obj->key_type = OBJ_TYPE_INT64;
            obj->key.int64_val = d;
        }
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_longlong(long long d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        if ( 2 == sizeof(long long) ){
            obj->obj_type = OBJ_TYPE_INT16;
            obj->value.int16_val  = d;
        } else if ( 4 == sizeof(long long) ){
            obj->obj_type = OBJ_TYPE_INT32;
            obj->value.int32_val = d;
        } else {
            obj->obj_type = OBJ_TYPE_INT64;
            obj->value.int64_val = d;
        }
    } else {
        obj->isKey = isKey;
        if ( 2 == sizeof(long long) ){
            obj->key_type = OBJ_TYPE_INT16;
            obj->key.int16_val  = d;
        } else if ( 4 == sizeof(long long) ){
            obj->key_type = OBJ_TYPE_INT32;
            obj->key.int32_val = d;
        } else {
            obj->key_type = OBJ_TYPE_INT64;
            obj->key.int64_val = d;
        }
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_unsigned_char(unsigned char d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }
    
    if (false == isKey){
        obj->obj_type = OBJ_TYPE_UINT8;
        obj->value.uint8_val = d;
    } else {
        obj->isKey = isKey;
        obj->key_type = OBJ_TYPE_UINT8;
        obj->key.uint8_val = d;
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_unsigned_short(unsigned short d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        if ( 2 == sizeof(unsigned short) ){
            obj->obj_type = OBJ_TYPE_UINT16;
            obj->value.uint16_val = d;
        } else if ( 4 == sizeof(unsigned short) ){
            obj->obj_type = OBJ_TYPE_UINT32;
            obj->value.uint32_val = d;
        } else {
            obj->obj_type = OBJ_TYPE_UINT64;
            obj->value.uint64_val = d;
        }
    } else {
        obj->isKey = isKey;
        if ( 2 == sizeof(unsigned short) ){
            obj->key_type = OBJ_TYPE_UINT16;
            obj->key.uint16_val = d;
        } else if ( 4 == sizeof(unsigned short) ){
            obj->key_type = OBJ_TYPE_UINT32;
            obj->key.uint32_val = d;
        } else {
            obj->key_type = OBJ_TYPE_UINT64;
            obj->key.uint64_val = d;
        }
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_unsigned_int(unsigned int d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        if ( 2 == sizeof(unsigned int) ){
            obj->obj_type = OBJ_TYPE_UINT16;
            obj->value.uint16_val = d; 
        } else if ( 4 == sizeof(unsigned int) ){
            obj->obj_type = OBJ_TYPE_UINT32;
            obj->value.uint32_val = d;
        } else {
            obj->obj_type = OBJ_TYPE_UINT64;
            obj->value.uint64_val = d;
        }
    } else {
        obj->isKey = isKey;
        if ( 2 == sizeof(unsigned int) ){
            obj->key_type = OBJ_TYPE_UINT16;
            obj->key.uint16_val = d; 
        } else if ( 4 == sizeof(unsigned int) ){
            obj->key_type = OBJ_TYPE_UINT32;
            obj->key.uint32_val = d;
        } else {
            obj->key_type = OBJ_TYPE_UINT64;
            obj->key.uint64_val = d;
        }
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_unsigned_long(unsigned long d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        if ( 2 == sizeof(unsigned long) ){
            obj->obj_type = OBJ_TYPE_UINT16;
            obj->value.uint16_val = d;
        } else if ( 4 == sizeof(unsigned long) ){
            obj->obj_type = OBJ_TYPE_UINT32;
            obj->value.uint32_val = d;
        } else {
            obj->obj_type = OBJ_TYPE_UINT64;
            obj->value.uint64_val = d;
        }
    } else {
        obj->isKey = isKey;
        if ( 2 == sizeof(unsigned long) ){
            obj->key_type = OBJ_TYPE_UINT16;
            obj->key.uint16_val = d;
        } else if ( 4 == sizeof(unsigned long) ){
            obj->key_type = OBJ_TYPE_UINT32;
            obj->key.uint32_val = d;
        } else {
            obj->key_type = OBJ_TYPE_UINT64;
            obj->key.uint64_val = d;
        }
    }

    return obj;

}/*}}}*/

static struct msgpk_object * msgpk_build_node_unsigned_longlong(unsigned long long d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        if ( 2 == sizeof(unsigned long long) ){
            obj->obj_type = OBJ_TYPE_UINT16;
            obj->value.uint16_val = d;
        } else if ( 4 == sizeof(unsigned long long) ){
            obj->obj_type = OBJ_TYPE_UINT32;
            obj->value.uint32_val = d;
        } else {
            obj->obj_type = OBJ_TYPE_UINT64;
            obj->value.uint64_val = d;
        }
    } else {
        obj->isKey = isKey;
        if ( 2 == sizeof(unsigned long long) ){
            obj->key_type = OBJ_TYPE_UINT16;
            obj->key.uint16_val = d;
        } else if ( 4 == sizeof(unsigned long long) ){
            obj->key_type = OBJ_TYPE_UINT32;
            obj->key.uint32_val = d;
        } else {
            obj->key_type = OBJ_TYPE_UINT64;
            obj->key.uint64_val = d;
        }
    }

    return obj;

}/*}}}*/

static struct msgpk_object * msgpk_build_node_nil(struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        obj->obj_type = OBJ_TYPE_NIL;
        obj->value.bin_val = 0;
    } else {
        obj->isKey = isKey;
        obj->key_type = OBJ_TYPE_NIL;
        obj->key.bin_val = 0;
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_true(struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        obj->obj_type = OBJ_TYPE_TRUE;
        obj->value.bool_val = true;
    } else {
        obj->isKey = isKey;
        obj->key_type = OBJ_TYPE_TRUE;
        obj->key.bool_val = true;
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_false(struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        obj->obj_type = OBJ_TYPE_FALSE;
        obj->value.bool_val = false;
    } else {
        obj->isKey = isKey;
        obj->key_type = OBJ_TYPE_FALSE;
        obj->key.bool_val = false;
    }
    
    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_float(float d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        obj->obj_type = OBJ_TYPE_FLOAT; 
        obj->value.float_val = d;
    } else {
        obj->isKey = isKey;
        obj->key_type = OBJ_TYPE_FLOAT; 
        obj->key.float_val = d;
    }

    return obj;
}/*}}}*/

static struct msgpk_object * msgpk_build_node_double(double d, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }
    
    if (false == isKey){
        obj->obj_type = OBJ_TYPE_DOUBLE;
        obj->value.double_val = d;
    } else {
        obj->isKey = isKey;
        obj->key_type = OBJ_TYPE_DOUBLE;
        obj->key.double_val = d;
    }

    return obj;

}/*}}}*/

static struct msgpk_object * msgpk_build_node_bin(ubyte_t * b, size_t len, struct msgpk_object *obj, bool_t isKey){/*{{{*/
    if( NULL == obj){
        return NULL;
    }

    if (false == isKey){
        obj->obj_type = OBJ_TYPE_BIN;
        obj->obj_len = len;
        obj->value.bin_val = (ubyte_t *) malloc(sizeof(ubyte_t)*len);
        memcpy(obj->value.bin_val, b, len);
    } else {
        obj->isKey = isKey;
        obj->key_len = len;
        obj->key_type = OBJ_TYPE_BIN;
        obj->key.bin_val = (ubyte_t *) malloc(sizeof(ubyte_t)*len);
        memcpy(obj->key.bin_val, b, len);
    }

    return obj;
}/*}}}*/



//PackTree
void msgpk_build_tree_string(struct tree_context *ctx, char* str, size_t len){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_string(str, len, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }

    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_char(struct tree_context *ctx, char d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_char(d, obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_short(struct tree_context *ctx, short d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_short(d, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }

    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_int(struct tree_context *ctx, int d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj)
        return;

    msgpk_build_node_int(d, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }

    ctx->node = obj;
}/*}}}*/

void  msgpk_build_tree_long(struct tree_context *ctx, long d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_long(d, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void  msgpk_build_tree_longlong(struct tree_context *ctx, long long d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;
    
    msgpk_build_node_longlong(d, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_unsigned_char(struct tree_context *ctx, unsigned char d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_unsigned_char(d, obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_unsigned_short(struct tree_context *ctx, unsigned short d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_unsigned_short(d, obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_unsigned_int(struct tree_context *ctx, unsigned int d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_unsigned_int(d, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_unsigned_long(struct tree_context *ctx, unsigned long d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_unsigned_long(d, obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_unsigned_longlong(struct tree_context *ctx, unsigned long long d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_unsigned_longlong(d, obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_nil(struct tree_context *ctx){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_nil(obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_true(struct tree_context *ctx){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_true(obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_false(struct tree_context *ctx){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_false(obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_float(struct tree_context *ctx, float d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_float(d, obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_double(struct tree_context *ctx, double d){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;

    msgpk_build_node_double(d, obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_bin(struct tree_context *ctx, ubyte_t * b, size_t len){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if ( NULL == obj )
        return;
    
    msgpk_build_node_bin(b, len, obj, false);
    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_tree_array_begin(struct tree_context *ctx){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    obj->obj_type = OBJ_TYPE_ARRAY;
    if (true == ctx->is_child){
        ctx->node->child = obj;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
    ctx->is_child = true;
    msgpk_build_push_stack(ctx->node);
}/*}}}*/

void msgpk_build_tree_array_end(struct tree_context *ctx, int len){/*{{{*/
    ctx->node = msgpk_build_pop_stack();
    ctx->node->obj_len = len;
}/*}}}*/

void msgpk_build_tree_map_begin(struct tree_context *ctx){/*{{{*/
    struct msgpk_object *obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    obj->obj_type = OBJ_TYPE_MAP;
    if (true == ctx->is_child){
        ctx->node->child = obj;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
    ctx->is_child = true;
    msgpk_build_push_stack(ctx->node);
}/*}}}*/

void msgpk_build_tree_map_end(struct tree_context *ctx, int len){/*{{{*/
    ctx->node = msgpk_build_pop_stack(); 
    ctx->node->obj_len = len;
}/*}}}*/

//PackMap
void msgpk_build_map_string_int(struct tree_context *ctx, char* k, size_t len,  int v){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_string(k, len, obj, true);
    msgpk_build_node_int(v, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_string_long(struct tree_context *ctx, char* k, size_t len,  long v){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_string(k, len, obj, true);
    msgpk_build_node_long(v, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_string_longlong(struct tree_context *ctx, char* k, size_t len,  long long v){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_string(k, len, obj, true);
    msgpk_build_node_longlong(v, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_string_float(struct tree_context *ctx, char* k, size_t len,  float v){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_string(k, len, obj, true);
    msgpk_build_node_float(v, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_string_double(struct tree_context *ctx, char* k, size_t len, double v){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_string(k, len, obj, true);
    msgpk_build_node_double(v, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_string_string(struct tree_context *ctx, char* k, size_t len,  char* v, size_t vlen){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_string(k, len, obj, true);
    msgpk_build_node_string(v, vlen, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_string_bin(struct tree_context *ctx, char* k, size_t len, ubyte_t *v, size_t vlen){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_string(k, len, obj, true);
    msgpk_build_node_bin(v, vlen, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/


void msgpk_build_map_int_int(struct tree_context *ctx, int k, int v){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_int(k, obj, true);
    msgpk_build_node_int(v, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_int_long(struct tree_context *ctx, int k, long v){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_int(k, obj, true);
    msgpk_build_node_long(v, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_int_longlong(struct tree_context *ctx, int k, long long v){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_int(k, obj, true);
    msgpk_build_node_longlong(v, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_int_float(struct tree_context *ctx, int k, float v){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_int(k, obj, true);
    msgpk_build_node_float(v, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_int_double(struct tree_context *ctx, int k, double v){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_int(k, obj, true);
    msgpk_build_node_double(v, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_int_string(struct tree_context *ctx, int k, char* v, size_t vlen){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_int(k, obj, true);
    msgpk_build_node_string(v, vlen, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

void msgpk_build_map_int_bin(struct tree_context *ctx, int k, ubyte_t *v, size_t vlen){/*{{{*/
    struct msgpk_object * obj = new_msgpk_object();
    if( NULL == obj){
        return;
    }

    msgpk_build_node_int(k, obj, true);
    msgpk_build_node_bin(v, vlen, obj, false);

    if(true == ctx->is_child){
        ctx->node->child = obj;
        ctx->is_child = false;
    } else {
        ctx->node->next = obj;
    }
    ctx->node = obj;
}/*}}}*/

//struct msgpk_object* PackNode_Ext(PackBuffer *pb, size_t len, int8_t type);

/*
 *Tree Operation
 */

//Query

struct msgpk_object * msgpk_tree_find_rule_node(struct msgpk_object *root , int argc, ...){//argc pairs of keyType, keyValue/*{{{*/
    va_list ap;
    int i;
    va_start(ap, argc);
    struct msgpk_object *obj = root;
    for(i=0; i<argc; i += 2){
        if(obj != NULL && NULL != obj->child)
            obj = obj->child;
        else
            return NULL;
        object_type keyType = va_arg(ap, object_type); 
        object_value keyValue = va_arg(ap, object_value);
        obj = msgpk_tree_hit_node(obj, keyType, keyValue);
    }

    return obj;
}/*}}}*/

//print
void msgpk_tree_print(struct msgpk_object *obj, int space){/*{{{*/
    if (NULL == obj){
        printf("tree print failed, obj is NULL\n");
        return;
    }
    msgpk_tree_print_blank(space);
    char a[100] = {0};
    char b[100] = {0};
    int len = 0;
    
    if(obj->isKey == true){
        switch(obj->key_type){
            case OBJ_TYPE_STR:
                snprintf(a, sizeof(a), "\"%s\" : ", obj->key.str_val);
                break;
            case OBJ_TYPE_INT8:
                snprintf(a, sizeof(a), "(int8)%d : ", obj->key.int8_val);
                break;
            case OBJ_TYPE_INT16:
                snprintf(a, sizeof(a), "(int16)%d : ", obj->key.int16_val);
                break;
            case OBJ_TYPE_INT32:
                snprintf(a, sizeof(a), "(int32)%d : ", obj->key.int32_val);
                break;
            case OBJ_TYPE_INT64:
                snprintf(a, sizeof(a), "(int64)%ld : ", obj->key.int64_val);
                break;
            case OBJ_TYPE_UINT8:
                snprintf(a, sizeof(a), "(uint8)%u : ", obj->key.uint8_val);
                break;
            case OBJ_TYPE_UINT16:
                snprintf(a, sizeof(a), "(uint16)%u : ", obj->key.uint16_val);
                break;
            case OBJ_TYPE_UINT32:
                snprintf(a, sizeof(a), "(uint32)%u : ", obj->key.uint32_val);
                break;
            case OBJ_TYPE_UINT64:
                snprintf(a, sizeof(a), "(uint64)%lu : ", obj->key.uint64_val);
                break;
            case OBJ_TYPE_POSITIVE_INT:
                snprintf(a, sizeof(a), "(fixInt)%u : ", obj->key.uint8_val);
                break;
            case OBJ_TYPE_NEGATIVE_INT:
                snprintf(a, sizeof(a), "(fixInt)%d : ", obj->key.int8_val);
                break;
            default:
                break;
        }

        printf("%s", a);
        len += strlen(a);
    }
                                                  
    switch(obj->obj_type){
        case OBJ_TYPE_STR:
            snprintf(b, sizeof(b), "\"%s\"", obj->value.str_val);
            break;
        case OBJ_TYPE_NIL:
            snprintf(b, sizeof(b), "(NULL)");
            break;
        case OBJ_TYPE_TRUE:
            snprintf(b, sizeof(b), "(TRUE:%d)", obj->value.bool_val);
            break;
        case OBJ_TYPE_FALSE:
            snprintf(b, sizeof(b), "(FALSE:%d)", obj->value.bool_val);
            break;
        case OBJ_TYPE_BIN:
            snprintf(b, sizeof(b), "(BIN)\"%s\"", obj->value.bin_val);
            break;
        case OBJ_TYPE_FLOAT:
            snprintf(b, sizeof(b), "(float)%f", obj->value.float_val);
            break;
        case OBJ_TYPE_DOUBLE:
            snprintf(b, sizeof(b), "(double)%lf", obj->value.double_val);
            break;
        case OBJ_TYPE_POSITIVE_INT:
            snprintf(b, sizeof(b), "(fixInt)%u", obj->value.uint8_val);
            break;
        case OBJ_TYPE_NEGATIVE_INT:
            snprintf(b, sizeof(b), "(fixInt)%d", obj->value.int8_val);
            break;
        case OBJ_TYPE_INT8:
            snprintf(b, sizeof(b), "(int8)%d", obj->value.int8_val);
            break;
        case OBJ_TYPE_INT16:
            snprintf(b, sizeof(b), "(int16)%d", obj->value.int16_val);
            break;
        case OBJ_TYPE_INT32:
            snprintf(b, sizeof(b), "(int32)%d", obj->value.int32_val);
            break;
        case OBJ_TYPE_INT64:
            snprintf(b, sizeof(b), "(int64)%ld", obj->value.int64_val);
            break;
        case OBJ_TYPE_UINT8:
            snprintf(b, sizeof(b), "(uint8)%u", obj->value.uint8_val);
            break;
        case OBJ_TYPE_UINT16:
            snprintf(b, sizeof(b), "(uint16)%u", obj->value.uint16_val);
            break;
        case OBJ_TYPE_UINT32:
            snprintf(b, sizeof(b), "(uint32)%u", obj->value.uint32_val);
            break;
        case OBJ_TYPE_UINT64:
            snprintf(b, sizeof(b),"(uint64)%lu", obj->value.uint64_val);
            break;
        case OBJ_TYPE_ARRAY:
            snprintf(b, sizeof(b), "Array(%d):", obj->obj_len);
            break;
        case OBJ_TYPE_MAP:
            snprintf(b, sizeof(b), "Map(%d):", obj->obj_len);
            break;
        case OBJ_TYPE_EXT:
            snprintf(b, sizeof(b), "EXT(%d):", obj->obj_len);
            break;
        default:
            break;
    }

    printf("%s", b);
    len += strlen(b);

    if(obj->child != NULL){
        printf("|\n");
        msgpk_tree_print(obj->child, space+len);
    }

    if(obj->next != NULL){
        printf("\n");
        msgpk_tree_print(obj->next, space);
    }
        
}/*}}}*/

void msgpk_tree_print_json(struct msgpk_object *obj){/*{{{*/
    if (NULL == obj){
        printf("json print failed, obj is NULL\n");
        return;
    }

    if(obj->isKey == true){
        switch(obj->key_type){
            case OBJ_TYPE_STR:
                printf("\"%s\" : ", obj->key.str_val);
                break;
            case OBJ_TYPE_INT8:
                printf("(int8)%d : ", obj->key.int8_val);
                break;
            case OBJ_TYPE_INT16:
                printf("(int16)%d : ", obj->key.int16_val);
                break;
            case OBJ_TYPE_INT32:
                printf("(int32)%d : ", obj->key.int32_val);
                break;
            case OBJ_TYPE_INT64:
                printf("(int64)%ld : ", obj->key.int64_val);
                break;
            case OBJ_TYPE_UINT8:
                printf("(uint8)%u : ", obj->key.uint8_val);
                break;
            case OBJ_TYPE_UINT16:
                printf("(uint16)%u : ", obj->key.uint16_val);
                break;
            case OBJ_TYPE_UINT32:
                printf("(uint32)%u : ", obj->key.uint32_val);
                break;
            case OBJ_TYPE_UINT64:
                printf("(uint64)%lu : ", obj->key.uint64_val);
                break;
            case OBJ_TYPE_POSITIVE_INT:
                printf("(fixInt)%u : ", obj->key.uint8_val);
                break;
            case OBJ_TYPE_NEGATIVE_INT:
                printf("(fixInt)%d : ", obj->key.int8_val);
                break;
            default:
                break;
        }
    }
                                                  
    switch(obj->obj_type){
        case OBJ_TYPE_STR:
            printf("\"%s\"", obj->value.str_val);
            break;
        case OBJ_TYPE_NIL:
            printf("(NULL)");
            break;
        case OBJ_TYPE_TRUE:
            printf("(TRUE:%d)", obj->value.bool_val);
            break;
        case OBJ_TYPE_FALSE:
            printf("(FALSE:%d)", obj->value.bool_val);
            break;
        case OBJ_TYPE_BIN:
            printf("(BIN)\"%s\"", obj->value.bin_val);
            break;
        case OBJ_TYPE_FLOAT:
            printf("(float)%f", obj->value.float_val);
            break;
        case OBJ_TYPE_DOUBLE:
            printf("(double)%lf", obj->value.double_val);
            break;
        case OBJ_TYPE_POSITIVE_INT:
            printf("(fixInt)%u", obj->value.uint8_val);
            break;
        case OBJ_TYPE_NEGATIVE_INT:
            printf("(fixInt)%d", obj->value.int8_val);
            break;
        case OBJ_TYPE_INT8:
            printf("(int8)%d", obj->value.int8_val);
            break;
        case OBJ_TYPE_INT16:
            printf("(int16)%d", obj->value.int16_val);
            break;
        case OBJ_TYPE_INT32:
            printf("(int32)%d", obj->value.int32_val);
            break;
        case OBJ_TYPE_INT64:
            printf("(int64)%ld", obj->value.int64_val);
            break;
        case OBJ_TYPE_UINT8:
            printf("(uint8)%u", obj->value.uint8_val);
            break;
        case OBJ_TYPE_UINT16:
            printf("(uint16)%u", obj->value.uint16_val);
            break;
        case OBJ_TYPE_UINT32:
            printf("(uint32)%u", obj->value.uint32_val);
            break;
        case OBJ_TYPE_UINT64:
            printf("(uint64)%lu", obj->value.uint64_val);
            break;
        case OBJ_TYPE_ARRAY:
            printf("Array(%d):", obj->obj_len);
            break;
        case OBJ_TYPE_MAP:
            printf("Map(%d):", obj->obj_len);
            break;
        case OBJ_TYPE_EXT:
            printf("EXT(%d):", obj->obj_len);
            break;
        default:
            break;
    }

    if(obj->child != NULL){
        if(obj->obj_type == OBJ_TYPE_ARRAY){
            printf("[");
            msgpk_tree_print_json(obj->child);
            printf("]");
        } else{
            printf("{");
            msgpk_tree_print_json(obj->child);
            printf("}");
        }
    }

    if(obj->next != NULL){
        printf(", ");
        msgpk_tree_print_json(obj->next);
    }
        
}/*}}}*/

//free tree
void msgpk_tree_free(struct msgpk_object *obj){/*{{{*/
    if (NULL == obj){
        printf("tree free failed, obj is NULL\n");
        return;
    }

    if (obj->isKey == true){
        if (OBJ_TYPE_STR == obj->key_type)
            free(obj->key.str_val);
        if (OBJ_TYPE_BIN == obj->key_type)
            free(obj->key.bin_val);
    }
                                                  
    if (OBJ_TYPE_STR == obj->obj_type)
        free(obj->value.str_val);
    if (OBJ_TYPE_BIN == obj->obj_type)
        free(obj->value.bin_val);

    if (obj->child != NULL)
        msgpk_tree_free(obj->child);

    if (obj->next != NULL)
        msgpk_tree_free(obj->next);

    free(obj);
}/*}}}*/

