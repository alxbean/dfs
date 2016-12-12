/*************************************************************************
    > File Name: logdb_debug.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Fri 25 Nov 2016 11:20:55 AM CST
************************************************************************/

#ifndef _LOGDB_DEBUG_H_
#define _LOGDB_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif
    #include <stdio.h>
    #include <syslog.h>

    #ifdef DEBUG
        #define logdb_debug(format, args...) fprintf(stderr, format, ##args)
    #else
        #define logdb_debug(format, args...) syslog(LOG_ERR, format, ##args)
    #endif

#ifdef __cplusplus
}
#endif
#endif

