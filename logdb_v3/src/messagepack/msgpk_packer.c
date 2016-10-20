/*************************************************************************
    > File : packer.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Mon 07 Mar 2016 07:23:11 AM UTC
 ************************************************************************/

#include <stdio.h>
#include "msgpk_packer.h"

#define MsgpkPackerInitBufferSize 8192

//ENDIAN_LITTLE
#define TAKE8_8(d)  ((uint8_t *)&d)[0]
#define TAKE8_16(d) ((uint8_t *)&d)[0]
#define TAKE8_32(d) ((uint8_t *)&d)[0]
#define TAKE8_64(d) ((uint8_t *)&d)[0]

//ENDIAN_BIG

//declare
static struct pack_buffer* new_pack_buffer();
static int free_pack_buffer(struct pack_buffer **ppb);
static int pack_append_buffer(struct pack_buffer *pb, const ubyte_t *buf, size_t len);
static void pack_message(struct pack_buffer *pb, struct msgpk_object *obj);

static struct pack_buffer* new_pack_buffer(){/*{{{*/
    struct pack_buffer * pb = (struct pack_buffer *) calloc(1, sizeof(struct pack_buffer)); 
    if( NULL == pb){
        perror("new pack_buffer:");
        return NULL;
    }

    return pb;
}/*}}}*/

static int free_pack_buffer(struct pack_buffer **ppb){/*{{{*/
    if (NULL == ppb){
        perror("free_pack_buffer:");
        return -1;
    }

    struct pack_buffer *dead_pb = *ppb;
    free(dead_pb->buffer);
    free(dead_pb);
    *ppb = NULL;

    return 0;
}/*}}}*/

static int pack_append_buffer(struct pack_buffer *pb, const ubyte_t *buf, size_t len){/*{{{*/
    if(pb->alloc - pb->off < len){
        size_t nsize = (pb->alloc) ? pb->alloc*2 : MsgpkPackerInitBufferSize;
        while(nsize < pb->off + len){
            size_t tmp_nsize = nsize * 2;
            if (tmp_nsize <= nsize){
                nsize = pb->off + len;
                break;
            }
            nsize = tmp_nsize;
        }
    
        ubyte_t *tmp = (ubyte_t *)realloc(pb->buffer, nsize);
        if(!tmp)
         return -1;

        pb->buffer = (ubyte_t*)tmp;
        pb->alloc = nsize;
    }

    memcpy(pb->buffer + pb->off, buf, len);
    pb->off += len;
    return 0;
}/*}}}*/

static void pack_message(struct pack_buffer *pb, struct msgpk_object *obj){/*{{{*/
    if(obj->isKey == true){
        switch(obj->key_type){
            case OBJ_TYPE_STR:
                msgpk_pack_string(pb, obj->key_len);
                msgpk_pack_string_body(pb, obj->key.str_val, obj->key_len);
                printf("(str_val)\"%s\" -> ", obj->key.str_val);
                break;
            case OBJ_TYPE_INT8:
                msgpk_pack_fixInt8(pb, obj->key.int8_val);
                printf("(int8)%d -> ", obj->key.int8_val);
                break;
            case OBJ_TYPE_INT16:
                msgpk_pack_fixInt16(pb, obj->key.int16_val);
                printf("(int16)%d -> ", obj->key.int16_val);
                break;
            case OBJ_TYPE_INT32:
                msgpk_pack_fixInt32(pb, obj->key.int32_val);
                printf("(int32)%d -> ", obj->key.int32_val);
                break;
            case OBJ_TYPE_INT64:
                msgpk_pack_fixInt64(pb, obj->key.int64_val);
                printf("(int64)%ld -> ", obj->key.int64_val);
                break;
            case OBJ_TYPE_UINT8:
                msgpk_pack_fixInt8_u(pb, obj->key.uint8_val);
                printf("(uint8)%u -> ", obj->key.uint8_val);
                break;
            case OBJ_TYPE_UINT16:
                msgpk_pack_fixInt16_u(pb, obj->key.uint16_val); 
                printf("(uint16)%u -> ", obj->key.uint16_val);
                break;
            case OBJ_TYPE_UINT32:
                msgpk_pack_fixInt32_u(pb, obj->key.uint32_val);
                printf("(uint32)%u -> ", obj->key.uint32_val);
                break;
            case OBJ_TYPE_UINT64:
                msgpk_pack_fixInt64_u(pb, obj->key.uint64_val);
                printf("(uint64)%lu -> ", obj->key.uint64_val);
                break;
            case OBJ_TYPE_POSITIVE_INT:
                msgpk_pack_fixnum_positive(pb, obj->key.uint8_val);
                printf("(fixInt)%u -> ", obj->key.uint8_val);
                break;
            case OBJ_TYPE_NEGATIVE_INT:
                msgpk_pack_fixnum_negative(pb, obj->key.int8_val);
                printf("(fixInt)%d -> ", obj->key.int8_val);
                break;
            default:
                break;
        }
    }
                                                  
    switch(obj->obj_type){
        case OBJ_TYPE_STR:
            msgpk_pack_string(pb, obj->obj_len);
            msgpk_pack_string_body(pb, obj->value.str_val, obj->obj_len);
            printf("(str_val)\"%s\"\n", obj->value.str_val);
            break;
        case OBJ_TYPE_NIL:
            msgpk_pack_nil(pb);
            printf("(NULL)\n");
            break;
        case OBJ_TYPE_TRUE:
            msgpk_pack_true(pb);
            printf("(TRUE:%d)\n", obj->value.bool_val);
            break;
        case OBJ_TYPE_FALSE:
            msgpk_pack_false(pb);
            printf("(FALSE:%d)\n", obj->value.bool_val);
            break;
        case OBJ_TYPE_BIN:
            msgpk_pack_bin(pb, obj->obj_len);
            msgpk_pack_bin_body(pb, obj->value.bin_val, obj->obj_len);
            printf("(BIN)\"%s\"\n", obj->value.bin_val);
            break;
        case OBJ_TYPE_FLOAT:
            msgpk_pack_float(pb, obj->value.float_val);
            printf("(float)%f\n", obj->value.float_val);
            break;
        case OBJ_TYPE_DOUBLE:
            msgpk_pack_double(pb, obj->value.double_val);
            printf("(double)%lf\n", obj->value.double_val);
            break;
        case OBJ_TYPE_POSITIVE_INT:
            msgpk_pack_fixnum_positive(pb, obj->value.uint8_val);
            printf("(fixInt)%u\n", obj->value.uint8_val);
            break;
        case OBJ_TYPE_NEGATIVE_INT:
            msgpk_pack_fixnum_negative(pb, obj->value.int8_val);
            printf("(fixInt)%d\n", obj->value.int8_val);
            break;
        case OBJ_TYPE_INT8:
            msgpk_pack_fixInt8(pb, obj->value.int8_val);
            printf("(int8)%d\n", obj->value.int8_val);
            break;
        case OBJ_TYPE_INT16:
            msgpk_pack_fixInt16(pb, obj->value.int16_val);
            printf("(int16)%d\n", obj->value.int16_val);
            break;
        case OBJ_TYPE_INT32:
            msgpk_pack_fixInt32(pb, obj->value.int32_val);
            printf("(int32)%d\n", obj->value.int32_val);
            break;
        case OBJ_TYPE_INT64:
            msgpk_pack_fixInt64(pb, obj->value.int64_val);
            printf("(int64)%ld\n", obj->value.int64_val);
            break;
        case OBJ_TYPE_UINT8:
            msgpk_pack_fixInt8_u(pb, obj->value.uint8_val);
            printf("(uint8)%u\n", obj->value.uint8_val);
            break;
        case OBJ_TYPE_UINT16:
            msgpk_pack_fixInt16_u(pb, obj->value.uint16_val); 
            printf("(uint16)%u\n", obj->value.uint16_val);
            break;
        case OBJ_TYPE_UINT32:
            msgpk_pack_fixInt32_u(pb, obj->value.uint32_val);
            printf("(uint32)%u\n", obj->value.uint32_val);
            break;
        case OBJ_TYPE_UINT64:
            msgpk_pack_fixInt64_u(pb, obj->value.uint64_val);
            printf("(uint64)%lu\n", obj->value.uint64_val);
            break;
        case OBJ_TYPE_ARRAY:
            msgpk_pack_array(pb, obj->obj_len);
            printf("Array: %d\n", obj->obj_len);
            break;
        case OBJ_TYPE_MAP:
            msgpk_pack_map(pb, obj->obj_len);
            printf("Map: %d\n", obj->obj_len);
            break;
        case OBJ_TYPE_EXT:
            msgpk_pack_ext(pb, obj->obj_len, 0x00);
            printf("EXT: %d\n", obj->obj_len);
            break;
        default:
            break;
    }

    if(obj->child != NULL){
        printf("child:\n");
        pack_message(pb, obj->child);
    }

    if(obj->next != NULL){
        printf("next:\n");
        pack_message(pb, obj->next);
    }
        
}/*}}}*/

//define

void msgpk_pack_string(struct pack_buffer *pb, size_t len){/*{{{*/
   if(len < 32){
       ubyte_t head = 0xa0 | (ubyte_t)len; 
       pack_append_buffer(pb, &TAKE8_8(head), 1); 
   } else if(len < 256){
       ubyte_t buf[2];
       buf[0] = 0xd9;
       buf[1] = ((ubyte_t *)&len)[0];
       pack_append_buffer(pb, buf, 2);
   } else if(len < 65536){
       ubyte_t buf[3];
       buf[0] = 0xda;
       buf[1] = ((ubyte_t *)&len)[1];
       buf[2] = ((ubyte_t *)&len)[0];
       pack_append_buffer(pb, buf, 3); 
   } else {
       ubyte_t buf[5];
       buf[0] = 0xdb;
       buf[1] = ((ubyte_t *)&len)[3];
       buf[2] = ((ubyte_t *)&len)[2];
       buf[3] = ((ubyte_t *)&len)[1];
       buf[4] = ((ubyte_t *)&len)[0];
       pack_append_buffer(pb, buf, 5); 
   }
}/*}}}*/

void msgpk_pack_string_body(struct pack_buffer *pb, const void* body, size_t len){/*{{{*/
    pack_append_buffer(pb, (const ubyte_t*)body, len);
}/*}}}*/

void msgpk_pack_fixnum_positive(struct pack_buffer *pb, uint8_t d){/*{{{*/
    pack_append_buffer(pb, &TAKE8_8(d), 1);
}/*}}}*/

void msgpk_pack_fixnum_negative(struct pack_buffer *pb, int8_t d){/*{{{*/
    pack_append_buffer(pb, &TAKE8_8(d), 1);
}/*}}}*/

void msgpk_pack_fixInt8_u(struct pack_buffer *pb, uint8_t d){/*{{{*/
    ubyte_t buf[2] = {0xcc, TAKE8_8(d)};
    pack_append_buffer(pb, buf, 2);
}/*}}}*/

void msgpk_pack_fixInt16_u(struct pack_buffer *pb, uint16_t d){/*{{{*/
    ubyte_t buf[3];
    buf[0] = 0xcd;
    buf[1] = ((ubyte_t *)&d)[1];
    buf[2] = ((ubyte_t *)&d)[0];
    pack_append_buffer(pb, buf, 3);
}/*}}}*/

void msgpk_pack_fixInt32_u(struct pack_buffer *pb, uint32_t d){/*{{{*/
    ubyte_t buf[5];
    buf[0] = 0xce;
    buf[1] = ((ubyte_t *)&d)[3];
    buf[2] = ((ubyte_t *)&d)[2];
    buf[3] = ((ubyte_t *)&d)[1];
    buf[4] = ((ubyte_t *)&d)[0];
    pack_append_buffer(pb, buf, 5);
}/*}}}*/

void msgpk_pack_fixInt64_u(struct pack_buffer *pb, uint64_t d){/*{{{*/
    ubyte_t buf[9];
    buf[0] = 0xce;
    buf[1] = ((ubyte_t *)&d)[7];
    buf[2] = ((ubyte_t *)&d)[6];
    buf[3] = ((ubyte_t *)&d)[5];
    buf[4] = ((ubyte_t *)&d)[4];
    buf[5] = ((ubyte_t *)&d)[3];
    buf[6] = ((ubyte_t *)&d)[2];
    buf[7] = ((ubyte_t *)&d)[1];
    buf[8] = ((ubyte_t *)&d)[0];
    pack_append_buffer(pb, buf, 9);
}/*}}}*/

void msgpk_pack_fixInt8(struct pack_buffer *pb, int8_t d){/*{{{*/
    ubyte_t buf[2] = {0xd0, TAKE8_8(d)};
    pack_append_buffer(pb, buf, 2);
}/*}}}*/

void msgpk_pack_fixInt16(struct pack_buffer *pb, int16_t d){/*{{{*/
    ubyte_t buf[3];
    buf[0] = 0xd1;
    buf[1] = ((ubyte_t *)&d)[1];
    buf[2] = ((ubyte_t *)&d)[0];
    pack_append_buffer(pb, buf, 3);
}/*}}}*/

void msgpk_pack_fixInt32(struct pack_buffer *pb, int32_t d){/*{{{*/
    ubyte_t buf[5];
    buf[0] = 0xd2;
    buf[1] = ((ubyte_t *)&d)[3];
    buf[2] = ((ubyte_t *)&d)[2];
    buf[3] = ((ubyte_t *)&d)[1];
    buf[4] = ((ubyte_t *)&d)[0];
    pack_append_buffer(pb, buf, 5);
}/*}}}*/

void msgpk_pack_fixInt64(struct pack_buffer *pb, int64_t d){/*{{{*/
    ubyte_t buf[9];
    buf[0] = 0xd3;
    buf[1] = ((ubyte_t *)&d)[7];
    buf[2] = ((ubyte_t *)&d)[6];
    buf[3] = ((ubyte_t *)&d)[5];
    buf[4] = ((ubyte_t *)&d)[4];
    buf[5] = ((ubyte_t *)&d)[3];
    buf[6] = ((ubyte_t *)&d)[2];
    buf[7] = ((ubyte_t *)&d)[1];
    buf[8] = ((ubyte_t *)&d)[0];
    pack_append_buffer(pb, buf, 9);
}/*}}}*/

void msgpk_pack_nil(struct pack_buffer *pb){/*{{{*/
    static const ubyte_t d = 0xc0;
    pack_append_buffer(pb, &d, 1);
}/*}}}*/

void msgpk_pack_true(struct pack_buffer *pb){/*{{{*/
    static const ubyte_t d = 0xc3;
    pack_append_buffer(pb, &d, 1);
}/*}}}*/

void msgpk_pack_false(struct pack_buffer *pb){/*{{{*/
    static const ubyte_t d = 0xc2;
    pack_append_buffer(pb, &d, 1);
}/*}}}*/

void msgpk_pack_float(struct pack_buffer *pb, float d){/*{{{*/
    ubyte_t buf[5];
    union {
        float f; uint32_t i;
    } mem;
    mem.f = d;
    buf[0] = 0xca;
    buf[1] = ((ubyte_t *)&mem.i)[3];
    buf[2] = ((ubyte_t *)&mem.i)[2];
    buf[3] = ((ubyte_t *)&mem.i)[1];
    buf[4] = ((ubyte_t *)&mem.i)[0];
    pack_append_buffer(pb, buf, 5);
}/*}}}*/

void msgpk_pack_double(struct pack_buffer *pb, double d){/*{{{*/
    ubyte_t buf[9];
    union{
        double f; uint64_t i;
    } mem;
    mem.f = d;
    buf[0] = 0xcb;
    buf[1] = ((ubyte_t *)&mem.i)[7];
    buf[2] = ((ubyte_t *)&mem.i)[6];
    buf[3] = ((ubyte_t *)&mem.i)[5];
    buf[4] = ((ubyte_t *)&mem.i)[4];
    buf[5] = ((ubyte_t *)&mem.i)[3];
    buf[6] = ((ubyte_t *)&mem.i)[2];
    buf[7] = ((ubyte_t *)&mem.i)[1];
    buf[8] = ((ubyte_t *)&mem.i)[0];
    pack_append_buffer(pb, buf, 9);
}/*}}}*/

void msgpk_pack_array(struct pack_buffer *pb, size_t n){/*{{{*/
    if(n < 16){
        ubyte_t d = 0x90 | (ubyte_t)n;
        pack_append_buffer(pb, &TAKE8_8(d), 1);
    } else if(n < 65536){
        ubyte_t buf[3];
        buf[0] = 0xdc;
        buf[1] = ((ubyte_t *)&n)[1];
        buf[2] = ((ubyte_t *)&n)[0];
        pack_append_buffer(pb, buf, 3);
    } else{
        ubyte_t buf[5];
        buf[0] = 0xdd;
        buf[1] = ((ubyte_t *)&n)[3];
        buf[2] = ((ubyte_t *)&n)[2];
        buf[3] = ((ubyte_t *)&n)[1];
        buf[4] = ((ubyte_t *)&n)[0];
        pack_append_buffer(pb, buf, 5);
    }
}/*}}}*/

void msgpk_pack_map(struct pack_buffer *pb, size_t n){/*{{{*/
    if(n < 16){
        ubyte_t d = 0x80 | (ubyte_t)n;
        pack_append_buffer(pb, &TAKE8_8(d), 1);
    } else if(n < 65536){
        ubyte_t buf[3];
        buf[0] = 0xde;
        buf[1] = ((ubyte_t *)&n)[1];
        buf[2] = ((ubyte_t *)&n)[0];
        pack_append_buffer(pb, buf, 3);
    } else{
        ubyte_t buf[5];
        buf[0] = 0xdf;
        buf[1] = ((ubyte_t *)&n)[3];
        buf[2] = ((ubyte_t *)&n)[2];
        buf[3] = ((ubyte_t *)&n)[1];
        buf[4] = ((ubyte_t *)&n)[0];
        pack_append_buffer(pb, buf, 5);
    }
}/*}}}*/

void msgpk_pack_bin(struct pack_buffer *pb, size_t len){/*{{{*/
    if(len < 256){
        ubyte_t buf[2];
        buf[0] = 0xc4;
        buf[1] = (uint8_t)len;
        pack_append_buffer(pb, buf, 2);
    } else if(len < 65536){
        ubyte_t buf[3];
        buf[0] = 0xc5;
        buf[1] = ((ubyte_t *)&len)[1];
        buf[2] = ((ubyte_t *)&len)[0];
        pack_append_buffer(pb, buf, 3);
    } else{
        ubyte_t buf[5];
        buf[0] = 0xc6;
        buf[1] = ((ubyte_t *)&len)[3];
        buf[2] = ((ubyte_t *)&len)[2];
        buf[3] = ((ubyte_t *)&len)[1];
        buf[4] = ((ubyte_t *)&len)[0];
        pack_append_buffer(pb, buf, 5);
    }
}/*}}}*/

void msgpk_pack_bin_body(struct pack_buffer *pb, const void *body, size_t len){/*{{{*/
    pack_append_buffer(pb, (const ubyte_t *) body, len);
}/*}}}*/

void msgpk_pack_ext(struct pack_buffer *pb, size_t len, int8_t type){/*{{{*/
    switch(len){
        case 1:{
            ubyte_t buf[2];
            buf[0] = 0xd4;
            buf[1] = type;
            pack_append_buffer(pb, buf, 2);
        } break;
        case 2:{
            ubyte_t buf[2];
            buf[0] = 0xd5;
            buf[1] = type;
            pack_append_buffer(pb, buf, 2);
        } break;
        case 4:{
            ubyte_t buf[2];
            buf[0] = 0xd6;
            buf[1] = type;
            pack_append_buffer(pb, buf, 2);
        } break;
        case 8:{
            ubyte_t buf[2];
            buf[0] = 0xd7;
            buf[1] = type;
            pack_append_buffer(pb, buf, 2);
        } break;
        case 16:{
            ubyte_t buf[2];
            buf[0] = 0xd8;
            buf[1] = type;
            pack_append_buffer(pb, buf, 2);
        } break;
        default:
            if(len <256){
                ubyte_t buf[3];
                buf[0] = 0xc7;
                buf[1] = (ubyte_t)len;
                buf[2] = type;
                pack_append_buffer(pb, buf, 3);
            } else if(len < 65536){
                ubyte_t buf[4];
                buf[0] = 0xc8;
                buf[1] = ((ubyte_t *)&len)[1];
                buf[2] = ((ubyte_t *)&len)[0];
                buf[3] = type;
                pack_append_buffer(pb, buf, 4);
            } else{
                ubyte_t buf[6];
                buf[0] = 0xc9;
                buf[1] = ((ubyte_t *)&len)[3];
                buf[2] = ((ubyte_t *)&len)[2];
                buf[3] = ((ubyte_t *)&len)[1];
                buf[4] = ((ubyte_t *)&len)[0];
                buf[5] = type;
                pack_append_buffer(pb, buf, 6);
            }
            break;
    }
}/*}}}*/

void msgpk_pack_ext_body(struct pack_buffer *pb, const void *body, size_t len){/*{{{*/
    pack_append_buffer(pb, (const ubyte_t *)body, len);
}/*}}}*/

struct pack_buffer *msgpk_message_packer(struct msgpk_object *obj){/*{{{*/
        /* creates buffer and serializer instance. */
        if (NULL == obj){
            printf("obj is NULL\n");
            return NULL;
        }

        struct pack_buffer *pb = new_pack_buffer(); 
        pack_message(pb, obj);

        return pb;
} /*}}}*/

int msgpk_message_destory(struct pack_buffer *pb){
    if (NULL == pb){
        perror("msgpk_message_destory:");
        return -1;
    }
    
    return free_pack_buffer(&pb);
}

void msgpk_print_hex(const ubyte_t *buf, unsigned int len){/*{{{*/
    size_t i = 0;
    for(; i < len; ++i)
       printf("0x%02x ", 0xff & buf[i]);
    printf("\n");
}/*}}}*/
