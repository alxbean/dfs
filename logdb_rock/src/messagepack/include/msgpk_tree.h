/*************************************************************************
  > File Name: msgpk_tree_query.h
  > Author: shuaixiang
  > Mail: shuaixiang@yuewen.com
  > Created Time: Mon 25 Apr 2016 07:24:17 AM UTC
 ************************************************************************/


#ifndef _MSGPK_TREE_H_
#define _MSGPK_TREE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "msgpk_define.h"
#include "skiplist.h"
#include "logdb_skp.h"

    struct spx_skp_serial_metadata_list *msgpk_tree_query(struct msgpk_object * obj);
    char * msgpk_tree_add(struct msgpk_object *root, size_t req_size, char *request);

#ifdef __cplusplus
}
#endif
#endif
