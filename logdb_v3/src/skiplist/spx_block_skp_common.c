/*************************************************************************
    > File Name: spx_block_skp_common.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Fri 28 Oct 2016 04:35:11 PM CST
************************************************************************/
#include "spx_block_skp_common.h"
#include "spx_message.h"
#include "spx_block_skp_serial.h"

/*
 * byte transform
 */

//int
ubyte_t* spx_block_skp_common_int2byte(const void* i, int* byte_len){/*{{{*/
    int b = *(int *)i;
    *byte_len = 4;
    ubyte_t *a = (ubyte_t *) malloc(sizeof(ubyte_t) * 4);
    *a = (b>>24) & 0xFF;
    *(a + 1) = (b>>16) & 0xFF;
    *(a + 2) = (b>>8)  & 0xFF;
    *(a + 3) = b & 0xFF;

    return a;
}/*}}}*/

void* spx_block_skp_common_byte2int(ubyte_t* b, int obj_len){/*{{{*/
    int *a = (int *) malloc(sizeof(int));
    *a = (((*b)<<24) | ((*(b + 1))<<16) | ((*(b + 2))<<8) | *(b + 3));
    return a;
}/*}}}*/

//int64_t
ubyte_t* spx_block_skp_common_long2byte(const void* i, int *byte_len){/*{{{*/
    int64_t n = *(int64_t *)i; 
    *byte_len = 8;
    ubyte_t *b = (ubyte_t *) malloc(sizeof(ubyte_t) * 8);//only for 64bit
    *b = (n >> 56) & 0xFF;
    *(b + 1) = (n >> 48) & 0xFF;
    *(b + 2) = (n >> 40) & 0xFF;
    *(b + 3) = (n >> 32) & 0xFF;
    *(b + 4) = (n >> 24) & 0xFF;
    *(b + 5) = (n >> 16) & 0xFF;
    *(b + 6) = (n >> 8) & 0xFF;
    *(b + 7) = n & 0xFF;

    return b;
}/*}}}*/

void *spx_block_skp_common_byte2long(ubyte_t *b, int obj_len){/*{{{*/
    int64_t *n = (int64_t *) malloc(sizeof(int64_t));
    *n =  (((int64_t) (*b)) << 56)
        | (((int64_t) (*(b+1))) << 48)
        | (((int64_t) (*(b + 2))) << 40)
        | (((int64_t) (*(b + 3))) << 32)
        | (((int64_t) (*(b + 4))) << 24)
        | (((int64_t) (*(b + 5))) << 16)
        | (((int64_t) (*(b + 6))) << 8)
        | ((int64_t) (*(b + 7)));

    return n;
}/*}}}*/

//float
ubyte_t *spx_block_skp_common_float2byte(const void * i, int *byte_len){/*{{{*/
    union {
        float f; uint32_t t;
    } fu;
    fu.f = *(float *)i; 
    *byte_len = 4;
    ubyte_t *b = (ubyte_t *) malloc(sizeof(ubyte_t) * 4);
    *b = ((ubyte_t *)&fu.t)[3];
    *(b + 1) = ((ubyte_t *)&fu.t)[2];
    *(b + 2) = ((ubyte_t *)&fu.t)[1];
    *(b + 3) = ((ubyte_t *)&fu.t)[0];

    return b;
}/*}}}*/

void *spx_block_skp_common_byte2float(ubyte_t *b, int obj_len){/*{{{*/
    union{
        float f; ubyte_t f_buf[4];
    } fu;

    int i = 0 ;
    for( i = 0; i < 4; i++){
        fu.f_buf[i] = *(b + 3 -i);
    }

    float * pf = (float *) malloc(sizeof(float));
    *pf = fu.f;

    return pf;
}/*}}}*/

//double
ubyte_t *spx_block_skp_common_double2byte(const void * i, int *byte_len){/*{{{*/
    union {
        double d; uint64_t t;
    } du;
    du.d = *(double *)i; 
    *byte_len = 8;
    ubyte_t *b = (ubyte_t *) malloc(sizeof(ubyte_t) * 8);
    *b = ((ubyte_t *)&du.t)[7];
    *(b + 1) = ((ubyte_t *)&du.t)[6];
    *(b + 2) = ((ubyte_t *)&du.t)[5];
    *(b + 3) = ((ubyte_t *)&du.t)[4];
    *(b + 4) = ((ubyte_t *)&du.t)[3];
    *(b + 5) = ((ubyte_t *)&du.t)[2];
    *(b + 6) = ((ubyte_t *)&du.t)[1];
    *(b + 7) = ((ubyte_t *)&du.t)[0];

    return b;
}/*}}}*/

void *spx_block_skp_common_byte2double(ubyte_t *b, int obj_len){/*{{{*/
    union{
        double d; ubyte_t d_buf[8];
    } du;

    int i = 0 ;
    for( i = 0; i < 8; i++){
        du.d_buf[i] = *(b + 7 -i);
    }

    double * pd = (double *) malloc(sizeof(double));
    *pd = du.d;

    return pd;
}/*}}}*/

//struct spx_skp_serial_metadata

ubyte_t * spx_block_skp_common_md2byte(const void * i, int *byte_len){/*{{{*/
    struct spx_skp_serial_metadata *md = (struct spx_skp_serial_metadata *)i; 
    return spx_skp_serial_m2b(md, byte_len);
}/*}}}*/

void *spx_block_skp_common_byte2md(ubyte_t *b, int obj_len){/*{{{*/
    return spx_skp_serial_b2m(b);
}/*}}}*/

//string
ubyte_t * spx_block_skp_common_str2byte(const void * i, int *byte_len){/*{{{*/
    *byte_len = strlen((char *)i);
    ubyte_t * buf = (ubyte_t*)malloc(sizeof(ubyte_t)*(*byte_len));
    strncpy((char*)buf, (char*) i, *byte_len);

    return buf;
}/*}}}*/

void *spx_block_skp_common_byte2str(ubyte_t *b, int obj_len){/*{{{*/
    char * buf = (char *)calloc(1, sizeof(char)*(obj_len + 1));
    strncpy(buf, (char*) b, obj_len);
    *(buf + obj_len) = '\0';
    return buf;
}/*}}}*/


/*
 *cmp method
 */

int cmp_md(const void *a, const void *b){/*{{{*/
    struct spx_skp_serial_metadata *mda = (struct spx_skp_serial_metadata *) a;
    struct spx_skp_serial_metadata *mdb = (struct spx_skp_serial_metadata *) b;

    if (strlen(mda->file) >  strlen(mdb->file))
        return 1;
    else if (strlen(mda->file) < strlen(mdb->file))
        return -1;
    else{
        if (strcmp(mda->file, mdb->file) > 0)
            return 1;
        else if (strcmp(mda->file, mdb->file) < 0)
            return -1;
        else{
            if (mda->off > mdb->off)
                return 1; 
            else if (mda->off < mdb->off)
                return -1;
            else{
                if (mda->len > mdb->len)
                    return 1;
                else if (mda->len < mdb->len)
                    return -1;
                else
                    return 0;
            }
        }
    }
}/*}}}*/
