/*************************************************************************
    > File Name: ydb_tracker_monitor.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Fri 27 Nov 2015 08:11:59 AM UTC
 ************************************************************************/
#ifndef _YDB_TRACKER_MONITOR_H_
#define _YDB_TRACKER_BALANCE_H_
#ifdef __cplusplus
extern "C"{
#endif

#include "../spx2/include/spx_types.h"
#include "../spx2/include/spx_io.h"
#include "ydb_tracker_configurtion.h"
#include "ydb_tracker_heartbeat.h"


pthread_t ydb_tracker_monitor_thread_new(SpxLogDelegate *log, struct ydb_tracker_configurtion *c, err_t *err);

spx_private void * ydb_tracker_monitor_mainsocket_create(void *arg);

spx_private void ydb_tracker_monitor_socket_accept_nb(SpxLogDelegate *log, struct ev_loop *loop, int fd);

spx_private void ydb_tracker_monitor_socket_main_reciver(struct ev_loop *loop, ev_io *watcher, int revents);

spx_private void * ydb_tracker_monitor_socket_main_response(void * client);

spx_private void bad_request(int client);

spx_private void headers(int client, int content_length);

spx_private void unimplemented(int client);

spx_private struct StorageList *ydb_tracker_visit_storage(string_t groupname, struct spx_job_context *jc);

spx_private void SendStorageJSON(int client, string_t groupname , struct spx_job_context *jc);

spx_private char * Storage2JSON(struct ydb_remote_storage * storage);

spx_private void FreeStorageList(struct StorageList *lit);

#ifdef _cplusplus
}
#endif
#endif
