/*************************************************************************
> File Name: msgpk_packer.h
> Author: shuaixiang
> Mail: shuaixiang@yuewen.com
> Created Time: Fri 11 Mar 2016 03:22:39 AM UTC
************************************************************************/

#ifndef _MSGPK_PACKER_H_
#define _MSGPK_PACKER_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "msgpk_define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

    struct pack_buffer{
        size_t off;
        ubyte_t *buffer;
        size_t alloc;
    }; 

    //Interfce
    struct pack_buffer *msgpk_message_packer(struct msgpk_object *obj);
    void msgpk_print_hex(const ubyte_t *buf, unsigned int len);

    //basic pack method
    void msgpk_pack_string(struct pack_buffer *pb, size_t len);
    void msgpk_pack_string_body(struct pack_buffer *pb, const void* body, size_t len);
    void msgpk_pack_fixnum_positive(struct pack_buffer *pb, uint8_t d);
    void msgpk_pack_fixnum_negative(struct pack_buffer *pb, int8_t d);
    void msgpk_pack_fixInt8_u(struct pack_buffer *pb, uint8_t d);
    void msgpk_pack_fixInt16_u(struct pack_buffer *pb, uint16_t d);
    void msgpk_pack_fixInt32_u(struct pack_buffer *pb, uint32_t d);
    void msgpk_pack_fixInt64_u(struct pack_buffer *pb, uint64_t d);
    void msgpk_pack_fixInt8(struct pack_buffer *pb, int8_t d);
    void msgpk_pack_fixInt16(struct pack_buffer *pb, int16_t d);
    void msgpk_pack_fixInt32(struct pack_buffer *pb, int32_t d);
    void msgpk_pack_fixInt64(struct pack_buffer *pb, int64_t d);
    void msgpk_pack_nil(struct pack_buffer *pb);
    void msgpk_pack_true(struct pack_buffer *pb);
    void msgpk_pack_false(struct pack_buffer *pb);
    void msgpk_pack_float(struct pack_buffer *pb, float d);
    void msgpk_pack_double(struct pack_buffer *pb, double d);
    void msgpk_pack_array(struct pack_buffer *pb, size_t n);
    void msgpk_pack_map(struct pack_buffer *pb, size_t n);
    void msgpk_pack_bin(struct pack_buffer *pb, size_t len);
    void msgpk_pack_bin_body(struct pack_buffer *pb, const void *body, size_t len);
    void msgpk_pack_ext(struct pack_buffer *pb, size_t len, int8_t type);
    void msgpk_pack_ext_body(struct pack_buffer *pb, const void *body, size_t len);
#ifdef __cplusplus
}
#endif
#endif
