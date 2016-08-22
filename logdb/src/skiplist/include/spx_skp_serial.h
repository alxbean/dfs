/************************************************************************
  > File Name: serial.h
  > Author: shuaixiang
  > Mail: shuaixiang@yuewen.com
  > Created Time: Tue 15 Sep 2015 02:37:28 AM UTC
 ************************************************************************/

#ifndef _SPX_SKP_SERIAL_H_
#define _SPX_SKP_SERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include "spx_skp.h"

#ifndef ubyte_t
#define ubyte_t unsigned char 
#endif
    typedef ubyte_t* (*SpxSkpO2BDelegate)(const void *, int *);
    typedef void* (*SpxSkpB2ODelegate)(ubyte_t *, int);

    struct spx_skp_serial_mf{
        ubyte_t* mapped;
        size_t size;    
        size_t distance;
    };


    //others
    struct spx_skp_serial_metadata{
        char filename_len;
        char *file;
        int64_t off;
        int64_t len;
    };

    struct spx_skp_serial_md_list_node{
        struct spx_skp_serial_metadata *md;
        struct spx_skp_serial_md_list_node *next_md;
    };

    struct spx_skp_serial_md_list{
        struct spx_skp_serial_md_list_node *head;
        struct spx_skp_serial_md_list_node *tail;
    };

    struct spx_skp_idx{
        char *index;
        char type;
        struct spx_skp_idx *next_idx;
    } spx_skp_idx_head;//read index.config 

    //serial skiplist
    int spx_skp_serial(struct spx_skp *sl, SpxSkpO2BDelegate key2byte, SpxSkpO2BDelegate value2byte);
    int spx_skp_unserial(struct spx_skp *sl, SpxSkpB2ODelegate byte2key, SpxSkpB2ODelegate byte2value);
    struct spx_skp_idx *spx_skp_read_config();
    struct spx_skp * spx_skp_add(SpxSkpCmpDelegate cmp_key, SpxSkpCmpDelegate cmp_value, SpxSkpB2ODelegate byte2key, SpxSkpB2ODelegate byte2value, const char * sl_name, SpxSkpFreeDelegate free_key, SpxSkpFreeDelegate free_value);
    struct spx_skp * spx_skp_get(char *skp_name);

    struct spx_skp_serial_md_list * spx_skp_serial_md_list_new();
    int spx_skp_serial_md_list_insert(struct spx_skp_serial_md_list * md_list, struct spx_skp_serial_metadata *md);
    int spx_skp_serial_md_list_free(struct spx_skp_serial_md_list * md_list);

    ubyte_t * spx_skp_serial_data_writer2byte(const ubyte_t *data, size_t len);
    struct spx_skp_serial_metadata* spx_skp_serial_data_writer2md(const ubyte_t *data, size_t len);
    ubyte_t * spx_skp_serial_data_reader(struct spx_skp_serial_metadata *md);
    void spx_skp_serial_free_md(struct spx_skp_serial_metadata *md);

    //byte operation
    //int
    ubyte_t *spx_skp_serial_int2byte(const void * i, int *byte_len);
    void *spx_skp_serial_byte2int(ubyte_t *b, int obj_len);

    //long
    ubyte_t *spx_skp_serial_long2byte(const void * i, int *byte_len);
    void *spx_skp_serial_byte2long(ubyte_t *b, int obj_len);

    //float
    ubyte_t *spx_skp_serial_float2byte(const void * i, int *byte_len);
    void *spx_skp_serial_byte2float(ubyte_t *b, int obj_len);

    //double
    ubyte_t *spx_skp_serial_double2byte(const void * i, int *byte_len);
    void *spx_skp_serial_byte2double(ubyte_t *b, int obj_len);

    //struct spx_skp_serial_metadata
    ubyte_t * spx_skp_serial_md2byte(const void * i, int *byte_len);
    void *spx_skp_serial_byte2md(ubyte_t *b, int obj_len);

    //str
    ubyte_t * spx_skp_serial_str2byte(const void * i, int *byte_len);
    void *spx_skp_serial_byte2str(ubyte_t *b, int obj_len);

    //gen_md_unid
    void spx_skp_serial_gen_unid(struct spx_skp_serial_metadata *md, char *unid, size_t size);

#ifdef __cplusplus
}
#endif
#endif
