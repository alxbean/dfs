/*************************************************************************
  > File Name: spx_block_skp_common.h
  > Author: shuaixiang
  > Mail: shuaixiang@yuewen.com
  > Created Time: Tue 12 Apr 2016 02:44:04 AM UTC
 ************************************************************************/

#ifndef _SPX_BLOCK_SKP_COMMON_H_
#define _SPX_BLOCK_SKP_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif
    #include "spx_types.h"
    #include "logdb_types.h"

    //transform method
    ubyte_t* spx_block_skp_common_int2byte(const void* i, int* byte_len);
    void *spx_block_skp_common_byte2int(ubyte_t* b, int obj_len);

    ubyte_t* spx_block_skp_common_long2byte(const void* i, int* byte_len);
    void *spx_block_skp_common_byte2long(ubyte_t* b, int obj_len);

    ubyte_t* spx_block_skp_common_float2byte(const void* i, int* byte_len);
    void *spx_block_skp_common_byte2float(ubyte_t* b, int obj_len);

    ubyte_t* spx_block_skp_common_double2byte(const void* i, int* byte_len);
    void *spx_block_skp_common_byte2double(ubyte_t* b, int obj_len);

    ubyte_t* spx_block_skp_common_md2byte(const void* i, int* byte_len);
    void *spx_block_skp_common_byte2md(ubyte_t* b, int obj_len);

    ubyte_t* spx_block_skp_common_str2byte(const void* i, int* byte_len);
    void* spx_block_skp_common_byte2str(ubyte_t* b, int obj_len);

    //cmp method
    int cmp_md(const void *a, const void *b);

#ifdef __cplusplus
}
#endif
#endif

