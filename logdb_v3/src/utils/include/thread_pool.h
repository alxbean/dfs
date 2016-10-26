/*************************************************************************
    > File Name: thread_pool.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Tue 27 Sep 2016 10:15:59 AM CST
 ************************************************************************/

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#ifdef __cplusplus
extern "C" {
#endif
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <pthread.h>
    #include "spx_types.h"

    //#ifndef bool_t
    //    typedef enum {
    //        false = 0,
    //        true = 1
    //    }bool_t;
    //#endif

    void thread_pool_init(int max_thread_num);
    int pool_task_add(void (*process) (void *arg), void *arg);
    int thread_pool_destroy();

#ifdef __cplusplus
}
#endif
#endif

