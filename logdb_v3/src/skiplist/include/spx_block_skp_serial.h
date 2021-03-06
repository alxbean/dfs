/************************************************************************
  > File Name: spx_block_skp_serial.h
  > Author: shuaixiang
  > Mail: shuaixiang@yuewen.com
  > Created Time: Tue 15 Sep 2015 02:37:28 AM UTC
 ************************************************************************/

#ifndef _SPX_BLOCK_SKP_SERIAL_H_
#define _SPX_BLOCK_SKP_SERIAL_H_

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
#include "spx_block_skp.h"
#include "spx_block_skp_common.h"

#define SpxSkpSerialFileNameSize 255 

#ifndef ubyte_t
#define ubyte_t unsigned char 
#endif
    typedef ubyte_t *(*SpxSkpO2BDelegate)(const void *, int *);
    typedef void *(*SpxSkpB2ODelegate)(ubyte_t *, int);

    struct spx_skp_serial_map_stat{
        ubyte_t *mapped;
        size_t size;    
        size_t distance;
    };

    //others
    struct spx_skp_serial_metadata{
        char file[SpxSkpSerialFileNameSize];
        int64_t off;
        int64_t len;
    };

    struct spx_skp_serial_metadata_list_node {
        struct spx_skp_serial_metadata *md;
        struct spx_skp_serial_metadata_list_node *next_md;
    };

    struct spx_skp_serial_metadata_list {
        struct spx_skp_serial_metadata_list_node *head;
        struct spx_skp_serial_metadata_list_node *tail;
    };

    struct spx_block_skp_serial_context{
        void *old_left_key;
        void *old_right_key;
        void *new_left_key;
        void *new_right_key;
    };

    //serial context
    struct spx_block_skp_serial_context* spx_block_skp_serial_context_new();
    int spx_block_skp_serial_context_free(struct spx_block_skp_serial_context **serial_ctx);

    //spx_block_skp_serial
    int64_t spx_block_skp_serial(struct spx_block_skp *block_skp, void *key, void * value, int64_t index, struct spx_block_skp_serial_context *serial_ctx);
    int spx_block_skp_serial_node_query(struct spx_block_skp *block_skp, void *key, int64_t index, struct spx_skp_query_result *result);
    int spx_block_skp_unserial(struct spx_block_skp *block_skp);

    //log persistence
    struct spx_skp_serial_metadata_list * spx_skp_serial_metadata_list_new();
    int spx_skp_serial_metadata_list_insert(struct spx_skp_serial_metadata_list * md_lst, struct spx_skp_serial_metadata *md);
    int spx_skp_serial_metadata_list_free(struct spx_skp_serial_metadata_list * md_lst, bool_t is_free_md);
    struct spx_skp_serial_metadata* spx_skp_serial_data_writer2md(const ubyte_t *data, size_t len);
    ubyte_t * spx_skp_serial_data_reader(struct spx_skp_serial_metadata *md);
    void spx_skp_serial_md_free(struct spx_skp_serial_metadata *md);
    struct spx_skp_serial_metadata *spx_skp_serial_md_copy(struct spx_skp_serial_metadata * src_md);

    //m2b, b2m
    ubyte_t *spx_skp_serial_m2b(struct spx_skp_serial_metadata *md, int *byteLen);
    struct spx_skp_serial_metadata *spx_skp_serial_b2m(ubyte_t *b);
    //gen_md_unid
    void spx_skp_serial_gen_unid(struct spx_skp_serial_metadata *md, char *unid, size_t size);


#ifdef __cplusplus
}
#endif
#endif
