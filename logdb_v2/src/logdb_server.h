/*************************************************************************
    > File Name: spx_web_server.h
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Fri 27 Nov 2015 08:11:59 AM UTC
 ************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "ev.h"
#include "spx_types.h"

struct log_header{
    u32_t version;
    u32_t protocol;
    u64_t req_size;
};

struct server_config{
    SpxLogDelegate *log;
    u64_t stacksize;
    string_t ip;
    i32_t port;
    u32_t timeout;
    u32_t retry_times;
    u32_t context_pool_size;
};

struct server_context{
    char life_cycle;
    int fd;
    ev_tstamp timeout;
    ev_async async_watcher;
    struct server_context *p_nextCTX;
    struct server_context *p_preCTX;

    //request
    size_t header_retry;
    struct log_header *header;
    size_t req_retry;
    size_t req_len;
    size_t req_size; 
    char *request;

    //response
    long (*ServerHandler)(size_t req_size, char *request,  char **response); 
    size_t resp_retry;
    size_t resp_size;
    size_t split_size;
    char *response;
    size_t resp_len;
};

//interface
void msg_print(char const* buf, unsigned int len);
err_t StartLogDB(long (*ServerHandler)(size_t req_size, char *request, char **response), struct server_config *conf);

#ifdef __cplusplus
}
#endif
