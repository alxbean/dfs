/*************************************************************************
    > File Name: logdb_types.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Thu 20 Oct 2016 10:36:55 AM CST
 ************************************************************************/

#ifndef _LOGDB_TYPES_H_
#define _LOGDB_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif
    #include <stdio.h>
    #include <stdlib.h>

    typedef int (*CmpDelegate)(const void *, const void *);
    typedef void (*FreeDelegate)(void *key);

    #ifndef bool_t
        typedef enum {
            false = 0,
            true = 1
        }bool_t;
    #endif

#ifdef __cplusplus
}
#endif
#endif
