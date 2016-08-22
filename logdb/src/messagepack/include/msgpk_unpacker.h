/*************************************************************************
> File Name: msgpk_unpacker.h
> Author: shuaixiang
> Mail: shuaixiang@yuewen.com
> Created Time: Fri 11 Mar 2016 03:22:39 AM UTC
************************************************************************/

#ifndef _MSGPK_UNPACKER_H_
#define _MSGPK_UNPACKER_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msgpk_define.h"

    struct context{
        struct msgpk_object *root;
        struct msgpk_object *node;
        ubyte_t *buf;
        int off;
    }; 

    //Interfce
    struct msgpk_object * msgpk_message_unpacker(ubyte_t *buf, int len);

#ifdef __cplusplus
}
#endif
#endif
