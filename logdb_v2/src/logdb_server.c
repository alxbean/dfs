/*************************************************************************
    > File Name: monkeyking.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Mon 23 Nov 2015 09:31:53 AM UTC
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <ev.h>
#include <ctype.h>

#include "spx_types.h"
#include "spx_message.h"
#include "spx_properties.h"
#include "spx_defs.h"
#include "spx_string.h"
#include "spx_socket.h"
#include "spx_io.h"
#include "spx_alloc.h"
#include "spx_module.h"
#include "spx_network_module.h"
#include "spx_job.h"
#include "spx_time.h"
#include "spx_log.h"
#include "spx_collection.h"
#include "logdb_server.h"

#define LOG_HEADER_SIZE (sizeof(u32_t)*2 + sizeof(u64_t)) 
#define SPLIT_SIZE 8192
#define RETRY_TIMES 3 
#define READ_HEADER 0
#define READ_REQUEST 1
#define PARSE_REQUEST 2
#define SEND_RESPONSE 3 

spx_private struct ev_loop *main_socket_loop = NULL;
spx_private ev_io monitor_watcher;
spx_private SpxLogDelegate *g_log = NULL;

spx_private struct server_context * ctx_top = NULL;
//Response Handler
spx_private void bad_request(int client);
spx_private void headers(int client, int content_length);
spx_private void unimplemented(int client);

//Server Service
spx_private pthread_t NewServerThread(SpxLogDelegate *log, struct server_config *conf, err_t *err);
spx_private void * CreateMainSocket(void *arg);
spx_private err_t NewCTXPool(struct server_config *conf, long (*ServerHandler)(size_t req_size, char *request, char **response));
spx_private void FreeCTXPool();
spx_private void Sender(EV_P_ ev_async *watcher, int revents);
spx_private void RegisterAayncWatcher(ev_async *watcher, void(*cb)(struct ev_loop *loop, ev_async *watcher, int revents), void *data);
spx_private void RegisterIOWatcher(ev_io *watcher ,int fd, void (*cb)(struct ev_loop *loop, ev_io *watcher, int revents), int revents, void *data);
spx_private void Reciver(struct ev_loop *loop, ev_io *watcher, int revents);
spx_private void ReciveRequest(struct ev_loop *loop, ev_async *watcher, int revents);
spx_private err_t ReciveRequest_GetRequest_ReadRequest(int fd, char *buf, size_t *len, size_t size);
spx_private void ReciveRequest_GetRequest(int revents, void *arg);
spx_private void ParserRequest(EV_P_ ev_async *watcher, int revents);//send as function pointer 
spx_private void SendResponse(EV_P_ ev_async *watcher, int revents);// send as function pointer
spx_private void RequestFinish(struct server_context *ctx);
spx_private void RequestException(struct server_context * ctx, void (*handle)(int client));
spx_private void CloseCTX(struct server_context *ctx);
spx_private void log_header_unpack(char *buf, struct log_header * header);

//tool
spx_private void log_header_unpack(char *buf, struct log_header * header){
   uchar_t *tmp = (uchar_t *) buf; 
   header->version = spx_msg_b2i(tmp); 
   header->protocol = spx_msg_b2i(tmp + sizeof(u32_t)); 
   header->req_size = spx_msg_b2l(tmp + sizeof(u32_t)*2);
}

void msg_print(char const* buf, unsigned int len)
{
    size_t i = 0;
    for(; i < len ; ++i)
        printf("%02x ", 0xff & buf[i]);
    printf("\n");
}

spx_private err_t CTXInit(struct server_context * ctx, long (*ServerHandler)(size_t req_size, char *request, char **response), struct server_config *conf){/*{{{*/
    err_t err = 0;
    if(NULL == ctx){
        SpxLog1(g_log, SpxLogError, "CTXInit ctx is NULL");
        return errno; 
    }

    ctx->p_nextCTX = NULL;
    ctx->p_preCTX = NULL;
    ctx->fd = 0;
    ctx->timeout = conf->timeout;
    ctx->life_cycle = 0;

    ctx->req_retry = 0;
    ctx->header_retry = 0;
    ctx->req_len = 0;
    ctx->req_size = 0;
    ctx->request = NULL;
    ctx->resp_retry = 0;
    ctx->resp_len = 0;   
    ctx->split_size = 0;
    ctx->response = NULL;
    ctx->ServerHandler = ServerHandler;

    ctx->header = (struct log_header *) calloc(1, sizeof(struct log_header));
    if(NULL== ctx->header){
        SpxLog2(g_log, SpxLogError, err, "calloc struct log_header failed");
        return err;
    }

    return err;
}/*}}}*/

spx_private struct server_context* CTXPop(){/*{{{*/
    struct server_context *ctx = ctx_top;
    if(NULL != ctx){
        ctx_top = ctx->p_preCTX;
        ctx_top->p_nextCTX = NULL;
        ctx->p_preCTX = NULL;
    }

    return ctx;
}/*}}}*/

spx_private err_t CTXPush(struct server_context * ctx){/*{{{*/
    err_t err = 0;

    if(ctx == NULL){
        perror("CTXPush NULL");
        err = errno;
    }else{
        ctx->life_cycle = 0;
        ctx->req_len = 0;
        ctx->resp_len = 0;
        ctx->header_retry = 0;
        ctx->req_retry = 0;
        ctx->resp_retry = 0;
        ctx->req_size = 0;
        ctx->resp_size = 0;
        ctx->split_size = 0;

        memset(ctx->header, 0, sizeof(struct log_header));
        free(ctx->request);
        ctx->request = NULL;
        free(ctx->response);
        ctx->response = NULL;

        if(NULL == ctx_top){
            ctx_top = ctx;
        }else{
            ctx_top->p_nextCTX = ctx;
            ctx->p_preCTX = ctx_top;
            ctx_top = ctx;
        }
    }
    
    return err;
}/*}}}*/

spx_private int GetCTXCount(){
    int count = 0;
    struct server_context *ctx = ctx_top;
    while(ctx != NULL){
        count++;
        ctx = ctx->p_preCTX;
    }

    return count;
}

spx_private struct server_context * GetCTXTop(){
    struct server_context *ctx = ctx_top;
    return ctx;
}

spx_private err_t NewCTXPool(struct server_config *conf, long (*ServerHandler)(size_t req_size, char *request, char **response)){/*{{{*/
   err_t err = 0; 
   uint32_t i = 0;

   for(i = 0; i < conf->context_pool_size; i++){
        struct server_context *ctx = (struct server_context *) malloc(sizeof(struct server_context));
        if(0 != CTXInit(ctx, ServerHandler, conf)){
           perror("NewCTXPool Failed");
           FreeCTXPool();
           err = errno;
           break;
        }else{
            if(NULL == ctx_top){
                ctx_top = ctx;
            }else{
                ctx_top->p_nextCTX = ctx;
                ctx->p_preCTX = ctx_top;
                ctx_top = ctx;
            }
        }
   }

  return err;
}/*}}}*/

spx_private void FreeCTXPool(){/*{{{*/
    while(NULL != ctx_top){
        struct server_context * ctx = ctx_top;
        ctx_top = ctx->p_preCTX;
        free(ctx->header);
        free(ctx);
    }
}/*}}}*/

spx_private void RegisterIOWatcher(ev_io *watcher ,int fd, void (*cb)(struct ev_loop *loop, ev_io *watcher, int revents), int revents, void *data){/*{{{*/
    SpxZero(*watcher);
    ev_io_init(watcher, cb, fd, revents);
    watcher->data = data;
}/*}}}*/

spx_private void RegisterAayncWatcher(ev_async *watcher, void(*cb)(struct ev_loop *loop, ev_async *watcher, int revents), void *data){/*{{{*/
    SpxZero(*watcher);
    ev_async_init(watcher, cb);
    watcher->data = data;
}/*}}}*/

spx_private err_t ReciveRequest_GetRequest_ReadRequest(int fd, char *buf, size_t *len, size_t size){/*{{{*/
    SpxErrReset;
    i64_t rc = 0;
    err_t err = 0;

    while(*len <= size){
        rc = read(fd, ((char *) buf) + *len, size - *len);
        if(0 > rc){
            err = errno;
            break;
        }else if(0 == rc){
            break;
        }else{
            *len += rc;
        }

    }

    return err;
}/*}}}*/

spx_private i64_t Sender_WriteResponse(int fd, char *buf, size_t *len, size_t *size){/*{{{*/
    SpxErrReset;
    i64_t rc = 0;
    err_t err = 0;

    rc = write(fd, ((char *) buf) + *len, *size);

    if(0 > rc){
        err = errno;
    }else{
        *len += rc;
        *size -= rc;
    }

    return err;
}/*}}}*/

spx_private void Sender_ReWriteResponse(int revents, void *arg){/*{{{*/
    struct server_context *ctx = (struct server_context *) arg;  
    if(NULL == ctx){
        SpxLog1(g_log, SpxLogError, "Sender ctx is NULL");
        return;
    }

    if(EV_ERROR & revents){
        SpxLog1(g_log, SpxLogError, "EV_ERROR");
        return;
    }

    if(EV_TIMEOUT & revents){
      if((ctx->resp_retry++) >= RETRY_TIMES){
        RequestException(ctx, bad_request);
        SpxLog1(g_log, SpxLogError, "EV_TIMEOUT");
        return;
      }else{
        ev_once(main_socket_loop, ctx->fd, EV_WRITE, ctx->timeout, Sender_ReWriteResponse, ctx);
        return;
      }
   }
  
   if(EV_WRITE & revents){
     err_t err = Sender_WriteResponse(ctx->fd, ctx->response, &ctx->resp_len, &ctx->split_size);        
     if((0 == err)&&(0 == ctx->split_size)){
          if(ctx->resp_size == ctx->resp_len){
                RequestFinish(ctx);             
          }else{
            int remain_size = ctx->resp_size - ctx->resp_len;
            if(remain_size >= SPLIT_SIZE){
                ctx->split_size = SPLIT_SIZE;
            }else{
                ctx->split_size = remain_size;
            }

            RegisterAayncWatcher(&ctx->async_watcher, Sender, ctx);
            ev_async_start(main_socket_loop, &ctx->async_watcher);
            ev_async_send(main_socket_loop, &ctx->async_watcher);
            return;
          }
     }else{
          if((EAGAIN == err || EWOULDBLOCK == err || EINTR == err)||(ctx->resp_size > 0)) {
              ev_once(main_socket_loop, ctx->fd, EV_READ, ctx->timeout, Sender_ReWriteResponse, ctx);
              return;
          }else{
             SpxLog2(g_log, SpxLogError, err,"Sender Failed");
             RequestException(ctx, bad_request);
          }
     }
   }
}/*}}}*/

spx_private void Sender(EV_P_ ev_async *watcher, int revents){/*{{{*/
    ev_async_stop(loop, watcher);
    struct server_context * ctx = (struct server_context *) watcher->data;
    if(NULL == ctx){
        SpxLog1(g_log, SpxLogError, "Sender ctx is NULL");
        return; 
    }

    err_t err = Sender_WriteResponse(ctx->fd, ctx->response, &ctx->resp_len, &ctx->split_size);        
    if((0 == err)&&(0 == ctx->split_size)){
         if(ctx->resp_size == ctx->resp_len){
            RequestFinish(ctx);             
         }else{
            int remain_size = ctx->resp_size - ctx->resp_len;
            if(remain_size >= SPLIT_SIZE){
                ctx->split_size = SPLIT_SIZE;
            }else{
                ctx->split_size = remain_size;
            }

            RegisterAayncWatcher(&ctx->async_watcher, Sender, ctx);
            ev_async_start(loop, &ctx->async_watcher);
            ev_async_send(loop, &ctx->async_watcher);
            return;
         }
    }else{
         if((EAGAIN == err || EWOULDBLOCK == err || EINTR == err)||(ctx->resp_size > 0)) {
             ev_once(main_socket_loop, ctx->fd, EV_READ, ctx->timeout, Sender_ReWriteResponse, ctx);
             return;
         }else{
           SpxLog2(g_log, SpxLogError, err,"Sender Failed");
           RequestException(ctx, bad_request);
         }
    }
}/*}}}*/

pthread_t NewServerThread(SpxLogDelegate *log, struct server_config *conf, err_t *err){/*{{{*/
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    size_t ostack_size =0;
    g_log = log;
    pthread_attr_getstacksize(&attr, &ostack_size);
    if ( (ostack_size != conf->stacksize) && (0 != (*err = pthread_attr_setstacksize(&attr, conf->stacksize)))){
         return 0;
    }

    if(NULL == conf){
        pthread_attr_destroy(&attr);
        return 0;
    }

    pthread_t tid = 0;
    if( 0 != (*err = pthread_create(&tid, &attr, CreateMainSocket, conf))){
        pthread_attr_destroy(&attr);
        return 0;
    }

    pthread_attr_destroy(&attr);
    return tid;
}/*}}}*/

spx_private void * CreateMainSocket(void *arg){/*{{{*/
    struct server_config * conf = (struct server_config *) arg;
    SpxLogDelegate *log = conf->log;
    err_t err = 0;
    
    main_socket_loop = ev_loop_new(0);
    if(NULL == main_socket_loop){
        SpxLog2(log, SpxLogError, err, "create main socket loop is fail.");
        goto r1;
    }

    int mainsocket = spx_socket_new(&err);
    if(0 == mainsocket){
        SpxLog2(log, SpxLogError, err, "create main socket is fail.");
        goto r1;
    }

    if( 0 != (err = spx_set_nb(mainsocket))){
        SpxLog2(log, SpxLogError, err, "set main socket nonblock is fail.");
    }

    if( 0 != (err = spx_socket_start(mainsocket, conf->ip, conf->port,\
                                     true, conf->timeout,\
                                     3, conf->timeout,\
                                     false, 0,\
                                     true,\
                                     true, conf->timeout,
                                     1024))){
        SpxLog2(log, SpxLogError, err, "start main socket is fail.");
        goto r1;
    }

    SpxLogFmt1(log, SpxLogMark,
               "main socket fd: %d"
               "and accepting...",
               mainsocket);

    RegisterIOWatcher(&monitor_watcher, mainsocket, Reciver, EV_READ, log);
    ev_io_start(main_socket_loop, &monitor_watcher); 

    ev_run(main_socket_loop, 0);
r1:
    SpxClose(mainsocket);
    return NULL;
}/*}}}*/

spx_private void Reciver(struct ev_loop *loop, ev_io *watcher, int revents){/*{{{*/
    ev_io_stop(loop, watcher);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    SpxLogDelegate *log = (SpxLogDelegate *) watcher->data;
    err_t err = 0;

    while(true){
        struct sockaddr_in client_addr;
        pthread_t tid = 0;
        unsigned int socket_len = 0;
        int client_sock = 0;
        socket_len = sizeof(struct sockaddr_in);
        client_sock = accept(watcher->fd, (struct sockaddr *) &client_addr, &socket_len);

        if (0 > client_sock){
            if( EWOULDBLOCK == errno || EAGAIN == errno){
                break;
            }
            break;
        }

        if( 0  == client_sock){
            break;
        }

        if( 0 != (err = spx_set_nb(client_sock))){
            SpxLog2(log, SpxLogError, err, "set main client_socket nonblock is fail.");
        }

        printf("\n----------------CLIENT:%d START CTX:%d-----------------------\n", client_sock, GetCTXCount());
        struct server_context * ctx = CTXPop();
        printf("\n----------------CLIENT:%d POP CTX:%d-----------------------\n", client_sock, GetCTXCount());

        if(NULL != ctx){
            ctx->fd = client_sock;
            ctx->life_cycle = READ_HEADER;
            RegisterAayncWatcher(&ctx->async_watcher, ReciveRequest, ctx);
            ev_async_start(loop, &ctx->async_watcher);
            ev_async_send(loop, &ctx->async_watcher);
        }

    }
      
    ev_io_start(loop, watcher);
}/*}}}*/

spx_private void ReciveRequest(EV_P_ ev_async *watcher, int revents){/*{{{*/
    ev_async_stop(loop, watcher);
    struct server_context * ctx = (struct server_context *) watcher->data;
    if(NULL == ctx){
        SpxLog1(g_log, SpxLogError, "ReciveRequest ctx is NULL");
        return; 
    }

    if(READ_HEADER == ctx->life_cycle){
        char buf[LOG_HEADER_SIZE] = {0};
        size_t len = 0;
        err_t err = ReciveRequest_GetRequest_ReadRequest(ctx->fd, buf, &len, LOG_HEADER_SIZE);
        if(0 == err){
            log_header_unpack(buf, ctx->header);
            char * request = (char *) calloc(1, sizeof(char)*ctx->header->req_size); 
            if(NULL == request){
                  SpxLog2(g_log, SpxLogError, err,"calloc log_header failed");
                  return;
            }

            ctx->req_size = ctx->header->req_size;
            ctx->request = request;
            ctx->life_cycle = READ_REQUEST;
        } else{
            if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                    ev_once(loop, ctx->fd, EV_READ, ctx->timeout, ReciveRequest_GetRequest, ctx);
                    return;
            } else{
                    SpxLog2(g_log, SpxLogError, err,"Read header failed");
                    CloseCTX(ctx);
                    return;
            }
        }
    }

    if(READ_REQUEST == ctx->life_cycle){
        printf("---------------ReadRequest-------------\n");
        printf("req_size:%d\n", (int)ctx->req_size);
        err_t err = ReciveRequest_GetRequest_ReadRequest(ctx->fd, ctx->request, &ctx->req_len, ctx->req_size);        
        printf("read request complete\n" );
        if(0 == err){
            ctx->life_cycle = PARSE_REQUEST;
            RegisterAayncWatcher(&ctx->async_watcher, ParserRequest, ctx);
            ev_async_start(loop, &ctx->async_watcher);
            ev_async_send(loop, &ctx->async_watcher);
            return;
        }else{
            if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                    ev_once(loop, ctx->fd, EV_READ, ctx->timeout, ReciveRequest_GetRequest, ctx);
                    return;
            }else{
                SpxLog2(g_log, SpxLogError, err,"ReciveRequest_GetRequest_ReadRequest Failed");
                CloseCTX(ctx);
                return;
            }
        }
    }
}/*}}}*/

spx_private void ReciveRequest_GetRequest(int revents, void *arg){/*{{{*/
    struct server_context *ctx = (struct server_context *) arg;
    if(NULL == ctx){
        SpxLog1(g_log, SpxLogError, "ReciveRequest_GetRequest ctx is NULL");
        return;
    }
    if(READ_HEADER == ctx->life_cycle){
         if(EV_ERROR & revents){
             SpxLog1(g_log, SpxLogError, "READ_HEADER EV_ERROR");
             return;
         }
         
         if(EV_TIMEOUT & revents){
           if((ctx->header_retry++) >= RETRY_TIMES){
             RequestException(ctx, bad_request);
             SpxLog1(g_log, SpxLogError, "READ_HEADER EV_TIMEOUT");
             return;
           } else{
             ev_once(main_socket_loop, ctx->fd, EV_READ, ctx->timeout, ReciveRequest_GetRequest, ctx);
             return;
           }
        }
  
        if(EV_READ & revents){
          char buf[LOG_HEADER_SIZE] = {0};
          size_t len = 0;
          err_t err = ReciveRequest_GetRequest_ReadRequest(ctx->fd, buf, &len, LOG_HEADER_SIZE);
          if(0 == err){
                log_header_unpack(buf, ctx->header);
                char * request = (char *) calloc(1, sizeof(char)*ctx->header->req_size); 
                if(NULL == request){
                      SpxLog2(g_log, SpxLogError, err,"calloc log_header failed");
                      return;
                }

                ctx->req_size = ctx->header->req_size;
                ctx->request = request;
                ctx->life_cycle = READ_REQUEST;
          } else{
               if((EAGAIN == err || EWOULDBLOCK == err || EINTR == err)) {
                   ev_once(main_socket_loop, ctx->fd, EV_READ, ctx->timeout, ReciveRequest_GetRequest, ctx);
                   return;
               } else{
                   SpxLog2(g_log, SpxLogError, err,"ReciveRequest_GetRequest_ReadRequest Failed");
                   CloseCTX(ctx);
                   return;
               }
          }
        }
    }

    if(READ_REQUEST == ctx->life_cycle){
         if(EV_ERROR & revents){
             SpxLog1(g_log, SpxLogError, "READ_REQUEST EV_ERROR");
             return;
         }
         
         if(EV_TIMEOUT & revents){
           if((ctx->req_retry++) >= RETRY_TIMES){
             RequestException(ctx, bad_request);
             SpxLog1(g_log, SpxLogError, "READ_REQUEST EV_TIMEOUT");
             return;
           } else{
             ev_once(main_socket_loop, ctx->fd, EV_READ, ctx->timeout, ReciveRequest_GetRequest, ctx);
             return;
           }
        }
  
        if(EV_READ & revents){
          err_t err = ReciveRequest_GetRequest_ReadRequest(ctx->fd, ctx->request, &ctx->req_len, ctx->req_size);        
          if(0 == err){
              ctx->life_cycle = PARSE_REQUEST;
              RegisterAayncWatcher(&ctx->async_watcher, ParserRequest, ctx);
              ev_async_start(main_socket_loop, &ctx->async_watcher);
              ev_async_send(main_socket_loop, &ctx->async_watcher);
          } else{
               if((EAGAIN == err || EWOULDBLOCK == err || EINTR == err)) {
                   ev_once(main_socket_loop, ctx->fd, EV_READ, ctx->timeout, ReciveRequest_GetRequest, ctx);
                   return;
               } else{
                   SpxLog2(g_log, SpxLogError, err,"ReciveRequest_GetRequest_ReadRequest Failed");
                   CloseCTX(ctx);
                   return;
               }
          }
        }
    }
}/*}}}*/

spx_private void ParserRequest(EV_P_ ev_async *watcher, int revents){/*{{{*/
    ev_async_stop(loop, watcher);
    err_t err = 0;
    struct server_context * ctx = (struct server_context *) watcher->data;
    if(NULL == ctx){
        SpxLog1(g_log, SpxLogError, "ParserRequest ctx is NULL");
        return; 
    }

   printf("\n----------------CLIENT:%d xxxxxxxxxxxxxxxxxx CTX:%d-----------------------\n", ctx->fd, GetCTXCount());
   if(NULL != ctx->request){
        //msg_print(ctx->request, ctx->req_size);
       
        long handle_size = ctx->ServerHandler(ctx->req_size, ctx->request, &ctx->response);
        if (-1 == handle_size){
            RequestException(ctx, bad_request);
        } else{
            ctx->resp_size = handle_size;
            ctx->life_cycle = SEND_RESPONSE;
            RegisterAayncWatcher(&ctx->async_watcher,SendResponse, ctx);
            ev_async_start(loop, &ctx->async_watcher);
            ev_async_send(loop, &ctx->async_watcher);
        }
   } else{
        RequestException(ctx, bad_request);
   }
}/*}}}*/

spx_private void SendResponse(EV_P_ ev_async *watcher, int revents){/*{{{*/
        ev_async_stop(loop, watcher);
        err_t err = 0;
        struct server_context * ctx = (struct server_context *) watcher->data;
        if(NULL == ctx){
            SpxLog1(g_log, SpxLogError, "SendResponse ctx is NULL");
            return; 
        }

        if(NULL != ctx->response){
            //headers(ctx->fd, ctx->resp_size);

            int remain_size = ctx->resp_size - ctx->resp_len;
            if(remain_size >= SPLIT_SIZE){
                ctx->split_size = SPLIT_SIZE;
            }else{
                ctx->split_size = remain_size;
            }

            RegisterAayncWatcher(&ctx->async_watcher, Sender, ctx);
            ev_async_start(loop, &ctx->async_watcher);
            ev_async_send(loop, &ctx->async_watcher);
        }else{
            RequestException(ctx, bad_request);
        }
}/*}}}*/

spx_private void RequestFinish(struct server_context *ctx){
        CloseCTX(ctx);
        printf("----------------CLIENT:%d SUCCESS CTX:%d-----------------------\n\n\n", ctx->fd, GetCTXCount());
}

spx_private void RequestException(struct server_context * ctx, void (*handle)(int client)){
        if (NULL == ctx){
            printf("ctx is NULL\n");
            return;
        }
        handle(ctx->fd);
        CloseCTX(ctx);
        printf("----------------CLIENT:%d FAILED CTX:%d-----------------------\n\n\n", ctx->fd, GetCTXCount());
}

spx_private void CloseCTX(struct server_context *ctx){
        if (NULL == ctx){
            printf("ctx is NULL\n");
            return;
        }

        SpxClose(ctx->fd);
        CTXPush(ctx);
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
/**********************************************************************/
spx_private void unimplemented(int client)/*{{{*/
{
 char msg[1024]={"unimplemented"};
 send(client, msg, strlen(msg), 0);
}/*}}}*/

/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
 *             the name of the file */
/**********************************************************************/
spx_private void headers(int client, int content_length)/*{{{*/
{
 char head[1024]={"header"};
 send(client, head, strlen(head), 0);
}/*}}}*/

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
/**********************************************************************/
spx_private void bad_request(int client)/*{{{*/
{
 char msg[1024]={"server error"};
 send(client, msg, strlen(msg), 0);
}/*}}}*/

err_t StartLogDB(long (*ServerHandler)(size_t req_size, char *request, char **response), struct server_config *conf){
    err_t err = 0;
    if(NULL == conf){
        SpxLog1(g_log,SpxLogError, "start server failed. server config is NULL");
        return -1;
    }

    if(0 != (err = NewCTXPool(conf, ServerHandler)) ){
        SpxLog1(g_log, SpxLogError, "NewCTXPool failed.");
        return err;
    }

    pthread_t tid = NewServerThread(g_log, conf, &err);
    if(0 != err){
        SpxLog2(g_log, SpxLogError, err,
                "create main socket thread is fail."
                "and will exit...");
        return err;
    }

    pthread_join(tid,NULL);
    return 0;
}
