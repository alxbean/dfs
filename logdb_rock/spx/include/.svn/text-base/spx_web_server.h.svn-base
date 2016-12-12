/*************************************************************************
    > File Name: spx_web_server.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Fri 27 Nov 2015 08:11:59 AM UTC
 ************************************************************************/
#ifndef SPX_WEB_SERVER_H
#define SPX_WEB_SERVER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ev.h"
#include "spx_types.h"

struct server_config{
    SpxLogDelegate *log;
    u64_t stacksize;
    string_t ip;
    i32_t port;
    u32_t timeout;
    u32_t retry_times;
    u32_t context_pool_size;
};

/*struct param_node{
    char key[100];
    char value[100];
    struct param_node *next_param;
};*/

struct server_context{
    char life_cycle;
    int fd;
    ev_tstamp timeout;
    ev_async async_watcher;
    struct server_context *p_nextCTX;
    struct server_context *p_preCTX;

    //request
    size_t req_retry;
    size_t req_len;
    size_t req_size; 
    char *request;
    struct param_node *param_head;
    struct param_node *param_tail;

    //response
    char *(*ServerHandler)(char *request); 
    size_t resp_retry;
    size_t resp_size;
    size_t split_size;
    char *response;
    size_t resp_len;
};

err_t StartWebServer(char *(*ServerHandler)(char *request), struct server_config *conf);

#ifdef __cplusplus
}
#endif
#endif
