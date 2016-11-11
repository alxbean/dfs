/*************************************************************************
    > File Name: logdb_skp_common.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Wed 02 Nov 2016 11:16:44 AM CST
************************************************************************/
#include "logdb_skp_common.h"
#include <string.h>

/*
 * public cmp method
 */


int cmp_int(const void *a, const void *b){/*{{{*/
    return *((int *)a) - *((int *)b);
}/*}}}*/

int cmp_long(const void *a, const void *b){/*{{{*/
    return *(long *)a - *(long *)b;
}/*}}}*/

int cmp_float(const void *a, const void *b){/*{{{*/
    return *(float *)a - *(float *)b;
}/*}}}*/

int cmp_double(const void *a, const void *b){/*{{{*/
    return *(double *)a - *(double *)b;
}/*}}}*/

int cmp_str(const void *a, const void *b){/*{{{*/
    return strcmp((char *)a, (char *)b);
}/*}}}*/


/*
 * public copy method
 */

 void* copy_int(void *a){/*{{{*/
     int *b = (int *) malloc(sizeof(int));
     *b = *a;
     return b;
 }/*}}}*/

 void* copy_long(void *a){/*{{{*/
     long *b = (long *) malloc(sizeof(long));
     *b = *a;
     return b;
 }/*}}}*/

 void* copy_float(void *a){/*{{{*/
     float *b = (float *) malloc(sizeof(float));
     *b = *a;
     return b;
 }/*}}}*/

 void* copy_double(void *a){/*{{{*/
     double *b = (double *) malloc(sizeof(double));
     *b = *a;
     return b;
 }/*}}}*/

 void* copy_str(void *a){/*{{{*/
     int len = strlen(a);
     char *b = (char *) malloc(sizeof(char) * len);
     strncpy(b, a, len);

     return b;
 }/*}}}*/
