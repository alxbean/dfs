/*************************************************************************
> File Name: msgpk_define.h
> Author: shuaixiang
> Mail: shuaixiang@yuewen.com
> Created Time: Mon 07 Mar 2016 08:05:25 AM UTC
************************************************************************/
#ifndef _MSGPK_DEFINE_H_
#define _MSGPK_DEFINE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "spx_types.h"
    //typedef unsigned char ubyte_t ;
    //typedef unsigned char byte_t ;
    //typedef char *string_t;
    //typedef enum{
    //    true = 1,
    //    false = 0
    //}bool_t;

    //static union{
    //    char a[4];
    //    unsigned long ul;
    //}endian={{'L','?', '?', 'B'}};

    //#define ENDIAN ((char)endian.ul)

    typedef enum{
        OBJ_TYPE_STR = 0x00,
        OBJ_TYPE_BIN = 0x01,
        OBJ_TYPE_INT8 = 0x02,
        OBJ_TYPE_UINT8 = 0x03,
        OBJ_TYPE_INT16 = 0x04,
        OBJ_TYPE_UINT16 = 0x05,
        OBJ_TYPE_INT32 = 0x06,
        OBJ_TYPE_UINT32 = 0x07,
        OBJ_TYPE_INT64 = 0x08,
        OBJ_TYPE_UINT64 = 0x09,
        OBJ_TYPE_FLOAT = 0x0A,
        OBJ_TYPE_DOUBLE = 0x0B,
        OBJ_TYPE_FALSE = 0X0C,
        OBJ_TYPE_TRUE = 0X0D,
        OBJ_TYPE_NIL = 0x0E,
        OBJ_TYPE_ARRAY = 0x0F,
        OBJ_TYPE_MAP = 0x10,
        OBJ_TYPE_EXT = 0x11,
        OBJ_TYPE_POSITIVE_INT = 0x12,
        OBJ_TYPE_NEGATIVE_INT = 0x13
    } object_type; 

    typedef union object_value{
        char *str_val;
        ubyte_t *bin_val;
        int8_t int8_val;
        uint8_t uint8_val;
        int16_t int16_val;
        uint16_t uint16_val;
        int32_t int32_val;
        uint32_t uint32_val;
        int64_t int64_val;
        uint64_t uint64_val;
        float float_val;
        double double_val;
        bool_t bool_val;
    } object_value;

    struct msgpk_object{
        bool_t isKey;
        object_type key_type;
        object_type obj_type;
        struct msgpk_object *next;
        struct msgpk_object *child;
        object_value value;
        int obj_len;
        int key_len;
        object_value key;
    };

    static inline struct msgpk_object * new_msgpk_object(){
        struct msgpk_object *new_node = (struct msgpk_object *) calloc(1, sizeof(struct msgpk_object));
        if(NULL == new_node){
            perror("new msgpk_object:");
            return NULL;
        }
    
        new_node->isKey = false;
        new_node->key_type = OBJ_TYPE_NIL;
        new_node->obj_type = OBJ_TYPE_NIL; 

        return new_node;
    }

#ifdef __cplusplus
}
#endif
#endif
