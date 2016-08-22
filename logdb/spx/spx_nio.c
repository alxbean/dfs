/*
 * =====================================================================================
 *
 *       Filename:  spx_nio.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/06/09 17时43分46秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <ev.h>

#include "spx_types.h"
#include "spx_job.h"
#include "spx_io.h"
#include "spx_defs.h"
#include "spx_nio.h"
#include "spx_errno.h"
#include "spx_message.h"
#include "spx_alloc.h"
#include "spx_task.h"


err_t  spx_nio_regedit_reader(struct ev_loop *loop,int fd,struct spx_job_context *jcontext){/*{{{*/
    //    if (NULL == jcontext) {
    //        return EINVAL;
    //    }
    //
    //    if (0 >= fd) {
    //        return EINVAL;
    //    }
    //    jcontext->lifecycle = SpxNioLifeCycleHeader;
    //    ev_io_init(&(jcontext->watcher),jcontext->nio_reader,fd,EV_READ);
    //    jcontext->watcher.data = jcontext;//libev not the set function
    //    ev_io_start(loop,&(jcontext->watcher));
    return 0;
}/*}}}*/

err_t  spx_nio_regedit_writer(struct ev_loop *loop,int fd,struct spx_job_context *jcontext){/*{{{*/
    //    if (NULL == jcontext) {
    //        return EINVAL;
    //    }
    //
    //    if (0 >= fd) {
    //        return EINVAL;
    //    }
    //
    //    jcontext->lifecycle = SpxNioLifeCycleHeader;
    //    ev_io_init(&(jcontext->watcher),jcontext->nio_writer,fd,EV_WRITE);
    //    jcontext->watcher.data = jcontext;
    //    ev_io_start(loop,&(jcontext->watcher));
    return 0;
}/*}}}*/

err_t  spx_dio_regedit_reader(struct ev_loop *loop,int fd,ev_io *watcher,
        SpxNioDelegate *dio_reader,void *data){/*{{{*/
    //    ev_io_init(watcher,dio_reader,fd,EV_READ);
    //    watcher->data = data;//libev not the set function
    //    ev_io_start(loop,watcher);
    return 0;
}/*}}}*/

err_t  spx_dio_regedit_writer(struct ev_loop *loop,int fd,ev_io *watcher,
        SpxNioDelegate *dio_writer,void *data){/*{{{*/
    //    ev_io_init(watcher,dio_writer,fd,EV_WRITE);
    //    watcher->data = data;
    //    ev_io_start(loop,watcher);
    return 0;
}/*}}}*/

err_t  spx_dio_regedit_async(ev_async *w,
        SpxAsyncDelegate *reader,void *data){/*{{{*/
    ev_async_init(w,reader);
    w->data = data;
    return 0;
}/*}}}*/

void spx_nio_reader(struct ev_loop *loop,ev_io *watcher,int revents){/*{{{*/
    //    if(NULL == loop || NULL == watcher){
    //        return ;
    //    }
    //    ev_io_stop(loop,watcher);
    //    size_t len = 0;
    //    err_t err = 0;
    //    struct spx_job_context *jcontext =(struct spx_job_context *) watcher->data;
    //    if(NULL == jcontext){
    //        return;
    //    }
    //
    //    if(EV_ERROR == revents){
    //        jcontext->err = 0 == errno ? EIO : errno;
    //        SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,
    //                "nio-read is fail,fd:%d,ip:%s.",
    //                jcontext->fd, jcontext->client_ip);
    //        goto r1;
    //    }
    //
    //    if(SpxNioLifeCycleHeader == jcontext->lifecycle){
    //        struct spx_msg *ctx = spx_msg_new(SpxMsgHeaderSize,&err);
    //        if(NULL == ctx){
    //            jcontext->err = err;
    //            SpxLogFmt2(jcontext->log,SpxLogError,err,
    //                    "alloc reader header msg is fail."
    //                    "client ip:%s.",
    //                    jcontext->client_ip);
    //            goto r1;
    //        }
    //        jcontext->reader_header_ctx = ctx;
    //        err = spx_read_to_msg_nb(watcher->fd,ctx,SpxMsgHeaderSize,&len);
    //        if(0 != err){
    //            jcontext->err = err;
    //            SpxLogFmt2(jcontext->log,SpxLogError,err,
    //                    "read header msg is fail."
    //                    "read len is %d."
    //                    "client ip:%s.",
    //                    len,
    //                    jcontext->client_ip);
    //            goto r1;
    //        }
    //        spx_msg_seek(ctx,0,SpxMsgSeekSet);
    //        struct spx_msg_header *header = spx_msg_to_header(ctx,&err);
    //        if(NULL == header){
    //            jcontext->err = err;
    //            SpxLogFmt2(jcontext->log,SpxLogError,err,
    //                    "convert msg-ctx to header is fail."
    //                    "client ip:%s.",
    //                    jcontext->client_ip);
    //            goto r1;
    //        }
    //        jcontext->reader_header = header;
    //        jcontext->lifecycle = SpxNioLifeCycleBody;
    //    }
    //
    //    if(NULL != jcontext->verifyHeaderHandler
    //            && !jcontext->verifyHeaderHandler(jcontext)){
    //        // must send msg to client
    //        jcontext->err = EBADHEADER;
    //        SpxLogFmt1(jcontext->log,SpxLogError,
    //                "validate header is fail."
    //                "and push jc force."
    //                "client:%s."
    //                "proto:%d,version:%d.",
    //                jcontext->client_ip,
    //                jcontext->reader_header->protocol,
    //                jcontext->reader_header->version);
    //        if(NULL != jcontext->reader_header_validator_fail){
    //            jcontext->reader_header_validator_fail(jcontext);
    //        }
    //        goto r1;
    //    }
    //
    //    if(NULL != jcontext->reader_body_process_before){
    //        jcontext->reader_body_process_before(jcontext);
    //    }
    //
    //    len = 0;
    //    if(SpxNioLifeCycleBody == jcontext->lifecycle) {
    //        jcontext->reader_body_process(loop,watcher->fd, jcontext);
    //        if(0 != jcontext->err){
    //            SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,
    //                    "deal body from client:%s by proto:%d is fail."
    //                    "and push jc to pool forcly.",
    //                    jcontext->client_ip,
    //                    jcontext->reader_header->protocol);
    //            return;
    //            //            goto r1;
    //        }
    //    }
    //    return;
    //r1:
    //    //    return;
    //    spx_job_pool_push(g_spx_job_pool,jcontext);
    return;
}/*}}}*/

void spx_nio_writer(struct ev_loop *loop,ev_io *watcher,int revents){/*{{{*/
    //    if(NULL == loop || NULL == watcher){
    //        return ;
    //    }
    //    ev_io_stop(loop,watcher);
    //    size_t len = 0;
    //    err_t err = 0;
    //    struct spx_job_context *jcontext =(struct spx_job_context *) watcher->data;
    //    if(NULL == jcontext){
    //        return;
    //    }
    //
    //    if(EV_ERROR == revents){
    //        jcontext->err = 0 == errno ? EIO : errno;
    //        SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,
    //                "nio-read is fail,fd:%d,ip:%s."
    //                "proto:%d,version:%d.",
    //                jcontext->fd, jcontext->client_ip,
    //                jcontext->writer_header->protocol,
    //                jcontext->writer_header->version);
    //        goto r1;
    //    }
    //
    //    if(SpxNioLifeCycleHeader == jcontext->lifecycle){
    //        struct spx_msg *ctx = spx_header_to_msg(jcontext->writer_header,SpxMsgHeaderSize,&err);
    //        if(NULL == ctx){
    //            jcontext->err = err;
    //            SpxLogFmt2(jcontext->log,SpxLogError,err,
    //                    "convert header to msg ctx is fail."
    //                    "proto:%d,version:%d."
    //                    "and forced push jcontext to pool.",
    //                    jcontext->writer_header->protocol,
    //                    jcontext->writer_header->version);
    //            goto r1;
    //        }
    //        jcontext->writer_header_ctx = ctx;
    //        err =  spx_write_from_msg_nb(watcher->fd,ctx,SpxMsgHeaderSize,&len);
    //        if(0 != err){
    //            jcontext->err = err;
    //            SpxLogFmt2(jcontext->log,SpxLogError,err,
    //                    "write writer header is fail.client:%s."
    //                    "len:%d."
    //                    "proto:%d,version:%d."
    //                    "and forced push jcontext to pool.",
    //                    jcontext->client_ip,len,
    //                    jcontext->writer_header->protocol,
    //                    jcontext->writer_header->version);
    //            goto r1;
    //        }
    //        jcontext->lifecycle = SpxNioLifeCycleBody;
    //    }
    //
    //    if(SpxNioLifeCycleBody == jcontext->lifecycle) {
    //        jcontext->writer_body_process(loop,watcher->fd,jcontext);
    //        if(0 != jcontext->err){
    //            SpxLogFmt2(jcontext->log,SpxLogError,jcontext->err,
    //                    "call write body handler to client:%s is fail."
    //                    "proto:%d,version:%d."
    //                    "and forced push jcontext to pool.",
    //                    jcontext->client_ip,
    //                    jcontext->writer_header->protocol,
    //                    jcontext->writer_header->version);
    //            return;
    //            //            goto r1;
    //        }
    //    }
    //    return;
    //r1:
    //    //    return;
    //    spx_job_pool_push(g_spx_job_pool,jcontext);
    return ;
}/*}}}*/

void spx_nio_reader_body_handler(struct ev_loop *loop,int fd,struct spx_job_context *jc){/*{{{*/
    if(0 == fd || NULL == jc){
        return;
    }
    if(SpxNioLifeCycleBody != jc->lifecycle){
        jc->err = ENXIO;
        SpxLogFmt1(jc->log,SpxLogError,
                "read body is fail and lifecycle is not body,fd:%d,ip:%s.",
                jc->fd, jc->client_ip);
        goto r1;
    }

    struct spx_msg_header *header = jc->reader_header;
    if(NULL == header){
        jc->err = ENXIO;
        SpxLogFmt1(jc->log,SpxLogError,
                "reader header is null."
                "client:%s.",
                jc->client_ip);
        goto r1;
    }
    if(0 == header->bodylen){
        jc->err = 0;
        SpxLogFmt1(jc->log,SpxLogError,
                "body len is 0 and no need read."
                "client:%s."
                "proto:%d,version:%d.",
                jc->client_ip,
                jc->reader_header->protocol,
                jc->reader_header->version);
        return;
    }

    size_t len = 0;
    if(jc->is_lazy_recv){
        struct spx_msg *ctx = spx_msg_new(header->offset,&(jc->err));
        if(NULL == ctx){
            SpxLogFmt2(jc->log,SpxLogError,jc->err,
                    "alloc body buffer is fail."
                    "client:%s."
                    "proto:%d,version:%d.",
                    jc->client_ip,
                    jc->reader_header->protocol,
                    jc->reader_header->version);
            goto r1;
        }
        jc->reader_body_ctx = ctx;
        jc->err = spx_read_to_msg_nb(fd,ctx,header->offset,&len);
        if(0 != jc->err){
            SpxLogFmt2(jc->log,SpxLogError,jc->err,\
                    "reader body is fail.bodylen:%lld,real body len:%lld.",
                    "client:%s."
                    "proto:%d,version:%d.",
                    header->offset,len,
                    jc->client_ip,
                    jc->reader_header->protocol,
                    jc->reader_header->version);
            goto r1;
        }
    } else {
        struct spx_msg *ctx = spx_msg_new(header->bodylen,&(jc->err));
        if(NULL == ctx){
            SpxLogFmt2(jc->log,SpxLogError,jc->err,
                    "alloc body buffer is fail.client:%s.",
                    "proto:%d,version:%d.",
                    jc->client_ip,
                    jc->reader_header->protocol,
                    jc->reader_header->version);
            goto r1;
        }
        jc->reader_body_ctx = ctx;
        jc->err = spx_read_to_msg_nb(fd,ctx,header->bodylen,&len);
        if(0 != jc->err){
            SpxLogFmt2(jc->log,SpxLogError,jc->err,
                    "reader body is fail.bodylen:%lld,real body len:%lld."
                    "client:%s."
                    "proto:%d,version:%d.",
                    header->bodylen,len,
                    jc->client_ip,
                    jc->reader_header->protocol,
                    jc->reader_header->version);
            goto r1;
        }
    }
    return;
r1:
    return;
    //    spx_job_pool_push(g_spx_job_pool,jc);
}/*}}}*/

void spx_nio_writer_body_handler(struct ev_loop *loop,int fd,struct spx_job_context *jc){/*{{{*/
    if(0 == fd || NULL == jc){
        return;
    }
    if(SpxNioLifeCycleBody != jc->lifecycle){
        jc->err = ENXIO;
        SpxLogFmt1(jc->log,SpxLogError,
                "write body is fail and lifecycle is not body,fd:%d,ip:%s.",
                jc->fd, jc->client_ip);
        goto r1;
    }

    if(NULL == jc->writer_header ) {
        jc->err = ENXIO;
        SpxLogFmt1(jc->log,SpxLogError,
                "header of write is null."
                "client ip:%s.",
                jc->client_ip);
        goto r1;
    }
    if( 0 == jc->writer_header->bodylen){
        SpxLogFmt1(jc->log,SpxLogWarn,\
                "the body length is 0 then no to be send."
                "client ip:%s."
                "proto:%d,version:%d.",
                jc->client_ip,
                jc->writer_header->protocol,
                jc->writer_header->version);
        return;
    }

    size_t len = 0;
    if(jc->is_sendfile){
        if(NULL != jc->writer_body_ctx && 0 != jc->writer_header->offset) {//no metedata
            jc->err =  spx_write_from_msg_nb(fd,jc->writer_body_ctx,\
                    jc->writer_header->offset,&len);
            if(0 != jc->err || jc->writer_header->offset != len){
                SpxLogFmt2(jc->log,SpxLogError,jc->err,\
                        "write  header to client :%s is fail.len:%d."
                        "proto:%d,version:%d."
                        "and forced push jc to pool.",
                        jc->client_ip,len,
                        jc->writer_header->protocol,
                        jc->writer_header->version);
                goto r1;
            }
        }
        size_t sendbytes = 0;
        jc->err = spx_sendfile(jc->fd,jc->sendfile_fd,\
                jc->sendfile_begin,jc->sendfile_size,&sendbytes);
        if(0 != jc->err || jc->sendfile_size != sendbytes){
            SpxLogFmt2(jc->log,SpxLogError,jc->err,\
                    "sendfile size:%lld is no equal sendbytes:%lld."
                    "client ip:%s."
                    "proto:%d,version:%d."
                    "and forced push jc to pool.",
                    jc->sendfile_size,sendbytes,
                    jc->client_ip,
                    jc->writer_header->protocol,
                    jc->writer_header->version);
            goto r1;
        }
    }else {
        jc->err =  spx_write_from_msg_nb(fd,jc->writer_body_ctx,\
                jc->writer_header->bodylen,&len);
        if(0 != jc->err || jc->writer_header->bodylen != len){
            SpxLogFmt2(jc->log,SpxLogError,jc->err,\
                    "write  header to client :%s is fail.len:%d."\
                    "proto:%d,version:%d."
                    "and forced push jc to pool.",\
                    jc->client_ip,len,
                    jc->writer_header->protocol,
                    jc->writer_header->version);
            goto r1;
        }
    }
    jc->lifecycle = SpxNioLifeCycleNormal;
    return;
r1:
    return;
    //    spx_job_pool_push(g_spx_job_pool,jc);
}/*}}}*/



//notice :
//this two faster function is not the best
//int thr version 2
//please open the errno(EAGIN and all of retry) to return
//and please regedit writer to loop when errno == eagin
//so faster means try direct write first if write is error,then use loop
//in the version 1,we just deal with error and over the write

void spx_nio_writer_faster(struct ev_loop *loop,int fd,struct spx_job_context *jc){/*{{{*/
    //    size_t len = 0;
    //    err_t err = 0;
    //    if(NULL == jc){
    //        return;
    //    }
    //
    //    if(SpxNioLifeCycleHeader == jc->lifecycle){
    //        struct spx_msg *ctx = spx_header_to_msg(jc->writer_header,SpxMsgHeaderSize,&err);
    //        if(NULL == ctx){
    //            jc->err = err;
    //            SpxLog2(jc->log,SpxLogError,err,
    //                    "convert writer header to msg ctx is fail."
    //                    "and forced push jc to pool.");
    //            goto r1;
    //        }
    //        jc->writer_header_ctx = ctx;
    //        err =  spx_write_from_msg_nb(fd,ctx,SpxMsgHeaderSize,&len);
    //        if(0 != err){
    //            jc->err = err;
    //            SpxLogFmt2(jc->log,SpxLogError,err,
    //                    "write writer header to client:%s is fail.len:%d."
    //                    "and forced push jc to pool.",
    //                    jc->client_ip,len);
    //            goto r1;
    //        }
    //        jc->lifecycle = SpxNioLifeCycleBody;
    //    }
    //
    //    len = 0;
    //    if(SpxNioLifeCycleBody == jc->lifecycle) {
    ////            && (0 != jc->writer_header->bodylen)){
    //        jc->writer_body_process(loop,fd,jc);
    //        if(0 != jc->err){
    //            SpxLogFmt2(jc->log,SpxLogError,jc->err,
    //                    "call write body handler to client:%s is fail."
    //                    "and forced push jc to pool.",
    //                    jc->client_ip);
    //            goto r1;
    //        }
    //    }
    //    return;
    //r1:
    //    return;
    //    //    spx_job_pool_push(g_spx_job_pool,jc);
    //
}/*}}}*/

//void spx_nio_writer_body_faster_handler(struct ev_loop *loop,int fd,struct spx_job_context *jc){/*{{{*/
//    if(0 == fd || NULL == jc){
//        return;
//    }
//    if(SpxNioLifeCycleBody != jc->lifecycle){
//        SpxLog1(jc->log,SpxLogError,
//                "the jc lifecycle is not body."
//                "and forced push jc to pool.");
//        goto r1;
//    }
//    size_t len = 0;
//    if(jc->is_sendfile){
//        jc->err =  spx_write_from_msg_nb(fd,jc->writer_body_ctx,
//                jc->writer_header->offset,&len);
//        if(0 != jc->err){
//            SpxLogFmt2(jc->log,SpxLogError,jc->err,
//                    "write  header to client :%s is fail.len:%d."
//                    "and forced push jc to pool.",
//                    jc->client_ip,len);
//            goto r1;
//        }
//        size_t sendbytes = 0;
//        jc->err = spx_sendfile(jc->fd,jc->sendfile_fd,
//                jc->sendfile_begin,jc->sendfile_size,&sendbytes);
//        if(0 != jc->err){
//            SpxLogFmt2(jc->log,SpxLogError,jc->err,
//                    "sndfile size:%lld is no equal sendbytes:%lld."
//                    "and forced push jc to pool.",
//                    jc->sendfile_size,sendbytes);
//            goto r1;
//        }
//    }else {
//        jc->err =  spx_write_from_msg_nb(fd,jc->writer_body_ctx,
//                jc->writer_header->bodylen,&len);
//        if(0 != jc->err){
//            SpxLogFmt2(jc->log,SpxLogError,jc->err,
//                    "write  header to client :%s is fail.len:%d."
//                    "and forced push jc to pool.",
//                    jc->client_ip,len);
//            goto r1;
//        }
//    }
//    jc->lifecycle = SpxNioLifeCycleNormal;
//    return;
//r1:
//    return;
//    //    spx_job_pool_push(g_spx_job_pool,jc);
//}/*}}}*/

void spx_nio_reader_with_timeout(int revents,void *arg){/*{{{*/
    struct spx_job_context *jc = (struct spx_job_context *) arg;
    if(NULL == jc) return;

    if (EV_ERROR & revents) {
        SpxLogFmt1(jc->log,SpxLogError,
                "waitting reading from client:%s is timeout,"
                "and close the client and push job-context to pool.",
                jc->client_ip);
        spx_job_pool_push(g_spx_job_pool,jc);
        return;
    }

    if(EV_TIMEOUT & revents) {
        if(jc->trytimes <= (jc->curr_trytimes++)){
            SpxLogFmt1(jc->log,SpxLogError,
                    "waitting client:%s to max times:%d."
                    "then close the client and push job-context to pool.",
                    jc->client_ip,jc->curr_trytimes);
            spx_job_pool_push(g_spx_job_pool,jc);
            return;
        } else {
            ev_once(jc->tc->loop,jc->fd,EV_READ,jc->waitting,jc->nio_reader,(void *) jc);
            SpxLogFmt1(jc->log,SpxLogWarn,
                    "re-try waitting client:%s again."
                    "current try-times is :%d.",
                    jc->client_ip,jc->curr_trytimes);
            return;
        }
    }

    err_t err = 0;
    if(EV_READ & revents){
        //        size_t len = 0;
        if(SpxNioLifeCycleHeader == jc->lifecycle){
            if(NULL == jc->reader_header_ctx) {
                struct spx_msg *ctx = spx_msg_new(SpxMsgHeaderSize,&err);
                if(NULL == ctx){
                    jc->err = err;
                    SpxLogFmt2(jc->log,SpxLogError,err,\
                            "alloc reader header msg is fail."
                            "client ip:%s.",
                            jc->client_ip);
                    spx_job_pool_push(g_spx_job_pool,jc);
                    return;
                }
                jc->reader_header_ctx = ctx;
            }

            err = spx_read_msg_ack(jc->fd,jc->reader_header_ctx,SpxMsgHeaderSize , &(jc->offset));
            if(err) {
                if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                    if(SpxMsgHeaderSize != jc->offset) {
                        ev_once(jc->tc->loop,jc->fd,EV_READ,jc->waitting,jc->nio_reader,(void *) jc);
                        return;
                    }
                } else {
                    jc->err = err;
                    SpxLogFmt2(jc->log,SpxLogError,err,\
                            "read header from client:%s is fail."
                            "header length:%d,real recv length:%d."
                            "then push job to pool.",
                            jc->client_ip,SpxMsgHeaderSize,jc->offset);
                    spx_job_pool_push(g_spx_job_pool,jc);
                    return;
                }
            }

            spx_msg_seek(jc->reader_header_ctx,0,SpxMsgSeekSet);
            struct spx_msg_header *header = spx_msg_to_header(jc->reader_header_ctx,&err);
            if(NULL == header){
                jc->err = err;
                SpxLogFmt2(jc->log,SpxLogError,err,
                        "convert msg-ctx to header is fail."
                        "client ip:%s.",
                        jc->client_ip);
                spx_job_pool_push(g_spx_job_pool,jc);
                return;
            }
            jc->reader_header = header;
            if(header->bodylen < header->offset){
                jc->err = EINVAL;
                SpxLogFmt1(jc->log,SpxLogError,
                        "body len is 0 and no need read."
                        "client:%s."
                        "proto:%d,version:%d.",
                        jc->client_ip,
                        jc->reader_header->protocol,
                        jc->reader_header->version);
                spx_job_pool_push(g_spx_job_pool,jc);
                return;
            }

            if(NULL != jc->verifyHeaderHandler) {
                // if validator return false,then the function must push jc to pool
                if(!jc->verifyHeaderHandler(jc)){
                    return;
                }
            }

            if(NULL != jc->beforeRecviveBodyHandler){
                // if body_process_before return false,then the function must push jc to pool
                if(!jc->beforeRecviveBodyHandler(jc)){
                    return;
                }
            }
            jc->lifecycle = SpxNioLifeCycleBody;
            jc->offset = 0;
            //            len = 0;
        }

        if(SpxNioLifeCycleBody == jc->lifecycle) {
            struct spx_msg_header *header = jc->reader_header;
            if(NULL == header){
                jc->err = ENXIO;
                SpxLogFmt1(jc->log,SpxLogError,
                        "reader header is null."
                        "client:%s.",
                        jc->client_ip);
                spx_job_pool_push(g_spx_job_pool,jc);
                return;
            }

            //only have header,the request is also dealed
            if(0 != header->offset || 0 != header->bodylen) {
                if(jc->is_lazy_recv){
                    if(NULL == jc->reader_body_ctx) {
                        struct spx_msg *ctx = spx_msg_new(header->offset,&(jc->err));
                        if(NULL == ctx){
                            SpxLogFmt2(jc->log,SpxLogError,jc->err,
                                    "alloc body buffer is fail."
                                    "client:%s."
                                    "proto:%d,version:%d.",
                                    jc->client_ip,
                                    jc->reader_header->protocol,
                                    jc->reader_header->version);
                            spx_job_pool_push(g_spx_job_pool,jc);
                            return;
                        }
                        jc->reader_body_ctx = ctx;
                    }

                    err = spx_read_msg_ack(jc->fd,jc->reader_body_ctx,jc->reader_header->offset , &(jc->offset));
                    if(err) {
                        if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                            //                        jc->offset += len;
                            if(header->offset != jc->offset) {
                                ev_once(jc->tc->loop,jc->fd,EV_READ,jc->waitting,jc->nio_reader,(void *) jc);
                                return;
                            }
                        } else {
                            jc->err = err;
                            SpxLogFmt2(jc->log,SpxLogError,err,\
                                    "read header from client:%s is fail."
                                    "body offset length:%d,real recv length:%d."
                                    "then push job to pool.",
                                    jc->client_ip,header->offset,jc->offset);
                            spx_job_pool_push(g_spx_job_pool,jc);
                            return;
                        }
                    }
                } else {
                    if(NULL == jc->reader_body_ctx) {
                        struct spx_msg *ctx = spx_msg_new(header->bodylen,&(jc->err));
                        if(NULL == ctx){
                            SpxLogFmt2(jc->log,SpxLogError,jc->err,
                                    "alloc body buffer is fail.client:%s.",
                                    "proto:%d,version:%d.",
                                    jc->client_ip,
                                    jc->reader_header->protocol,
                                    jc->reader_header->version);
                            spx_job_pool_push(g_spx_job_pool,jc);
                            return;
                        }
                        jc->reader_body_ctx = ctx;
                    }

                    err = spx_read_msg_ack(jc->fd,jc->reader_body_ctx,header->bodylen ,&(jc->offset));
                    if(err) {
                        if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                            //                        jc->offset += len;
                            if(header->bodylen != jc->offset) {
                                ev_once(jc->tc->loop,jc->fd,EV_READ,jc->waitting,jc->nio_reader,(void *) jc);
                                return;
                            }
                        } else {
                            jc->err = err;
                            SpxLogFmt2(jc->log,SpxLogError,err,\
                                    "read header from client:%s is fail."
                                    "body length:%d,real recv length:%d."
                                    "then push job to pool.",
                                    jc->client_ip,header->bodylen,jc->offset);
                            spx_job_pool_push(g_spx_job_pool,jc);
                            return;
                        }
                    }
                }
            }
            if(NULL != jc->disposeReaderHandler){
                jc->disposeReaderHandler(jc);
            }
        }
    }
}/*}}}*/

void spx_nio_writer_with_timeout(int revents,void *arg){/*{{{*/
    struct spx_job_context *jc = (struct spx_job_context *) arg;
    if(NULL == jc) return;

    if (EV_ERROR & revents) {
        SpxLogFmt1(jc->log,SpxLogError,
                "waitting writing to client:%s is fail,"
                "and close the client and push job-context to pool.",
                jc->client_ip);
        spx_job_pool_push(g_spx_job_pool,jc);
        return;
    }

    if(EV_TIMEOUT & revents) {
        if(jc->trytimes <= (jc->curr_trytimes++)){
            SpxLogFmt1(jc->log,SpxLogError,
                    "waitting writeting to client:%s to max times:%d."
                    "then close the client and push job-context to pool.",
                    jc->client_ip,jc->curr_trytimes);
            spx_job_pool_push(g_spx_job_pool,jc);
            return;
        } else {
            ev_once(jc->tc->loop,jc->fd,EV_WRITE,jc->waitting,jc->nio_writer,(void *) jc);
            SpxLogFmt1(jc->log,SpxLogWarn,
                    "re-try waitting writing to client:%s again."
                    "current try-times is :%d.",
                    jc->client_ip,jc->curr_trytimes);
            return;
        }
    }

    err_t err = 0;
    if(EV_WRITE & revents) {
        if(SpxNioLifeCycleHeader == jc->lifecycle){
            if(NULL == jc->writer_header_ctx) {
                struct spx_msg *ctx = spx_header_to_msg(jc->writer_header,SpxMsgHeaderSize,&err);
                if(NULL == ctx){
                    jc->err = err;
                    SpxLog2(jc->log,SpxLogError,err,\
                            "convert writer header to msg ctx is fail."\
                            "and forced push jc to pool.");
                    spx_job_pool_push(g_spx_job_pool,jc);
                    return;
                }
                jc->writer_header_ctx = ctx;
            }

            err = spx_write_msg_ack(jc->fd,jc->writer_header_ctx,&(jc->offset));
            if(err) {
                if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                    if(SpxMsgHeaderSize != jc->offset) {
                        ev_once(jc->tc->loop,jc->fd,EV_WRITE,jc->waitting,jc->nio_writer,(void *) jc);
                        return;
                    }
                } else {
                    jc->err = err;
                    SpxLogFmt2(jc->log,SpxLogError,jc->err,
                            "write  header to client :%s is fail.len:%d."
                            "proto:%d,version:%d."
                            "and forced push jc to pool.",
                            jc->client_ip,jc->offset,
                            jc->writer_header->protocol,
                            jc->writer_header->version);
                    spx_job_pool_push(g_spx_job_pool,jc);
                    return;
                }
            }
            jc->offset = 0;
            jc->lifecycle = SpxNioLifeCycleBody;
        }

        if(SpxNioLifeCycleBody == jc->lifecycle) {
            if(NULL != jc->writer_body_ctx  && 0 != jc->writer_header->bodylen) {//no metedata
                size_t len = 0 == jc->writer_header->offset ? jc->writer_header->bodylen : jc->writer_header->offset;
                err = spx_write_msg_ack(jc->fd,jc->writer_body_ctx,&(jc->offset));
                if(err) {
                    if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                        if(len != jc->offset) {
                            ev_once(jc->tc->loop,jc->fd,EV_WRITE,jc->waitting,jc->nio_writer,(void *) jc);
                            return;
                        }
                    } else {
                        jc->err = err;
                        SpxLogFmt2(jc->log,SpxLogError,jc->err,\
                                "write  body or prebody to client :%s is fail.len:%d."
                                "proto:%d,version:%d."
                                "and forced push jc to pool.",
                                jc->client_ip,len,
                                jc->writer_header->protocol,
                                jc->writer_header->version);
                        spx_job_pool_push(g_spx_job_pool,jc);
                        return;
                    }
                }
            }
            jc->lifecycle = SpxNioLifeCycleFile;
            jc->offset = 0;
        }

        if(SpxNioLifeCycleFile == jc->lifecycle){
            if(jc->is_sendfile){
                err = spx_sendfile_ack(jc->fd,jc->sendfile_fd,\
                        jc->sendfile_begin + jc->offset,jc->sendfile_size - jc->offset,&(jc->offset));
                if(err) {
                    if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                        if(jc->sendfile_size != jc->offset) {
                            ev_once(jc->tc->loop,jc->fd,EV_WRITE,jc->waitting,jc->nio_writer,(void *) jc);
                            return;
                        }
                    } else {
                        jc->err = err;
                        SpxLogFmt2(jc->log,SpxLogError,jc->err,
                                "sendfile to client :%s is fail.len:%d."
                                "proto:%d,version:%d."
                                "and forced push jc to pool.",
                                jc->client_ip,jc->offset,
                                jc->writer_header->protocol,
                                jc->writer_header->version);
                        spx_job_pool_push(g_spx_job_pool,jc);
                        return;
                    }
                }
            }
            jc->lifecycle = SpxNioLifeCycleNormal;
        }

        if(NULL != jc->disposeWriterHandler){
            jc->disposeWriterHandler(jc);
        }
        return;
    }
}/*}}}*/

void spx_nio_reader_with_timeout_fast(int revents,void *arg){/*{{{*/
    struct spx_job_context *jc = (struct spx_job_context *) arg;
    if(NULL == jc) return;

    if (EV_ERROR & revents) {
        SpxLogFmt1(jc->log,SpxLogError,
                "waitting reading from client:%s is timeout,"
                "and close the client and push job-context to pool.",
                jc->client_ip);
        spx_job_context_clear(jc);
        return;
    }

    if(EV_TIMEOUT & revents) {
        if(jc->trytimes <= (jc->curr_trytimes++)){
            SpxLogFmt1(jc->log,SpxLogError,
                    "waitting client:%s to max times:%d."
                    "then close the client and push job-context to pool.",
                    jc->client_ip,jc->curr_trytimes);
            spx_job_context_clear(jc);
            return;
        } else {
            ev_once(jc->tc->loop,jc->fd,EV_READ,jc->waitting,jc->nio_reader,(void *) jc);
            SpxLogFmt1(jc->log,SpxLogWarn,
                    "re-try waitting client:%s again."
                    "current try-times is :%d.",
                    jc->client_ip,jc->curr_trytimes);
            return;
        }
    }

    err_t err = 0;
    if(EV_READ & revents){
        if(SpxNioLifeCycleHeader == jc->lifecycle){
            if(NULL == jc->reader_header_ctx) {
                struct spx_msg *ctx = spx_msg_new(SpxMsgHeaderSize,&err);
                if(NULL == ctx){
                    jc->err = err;
                    SpxLogFmt2(jc->log,SpxLogError,err,\
                            "alloc reader header msg is fail."
                            "client ip:%s.",
                            jc->client_ip);
                    spx_job_context_clear(jc);
                    return;
                }
                jc->reader_header_ctx = ctx;
            }

            err = spx_read_msg_ack(jc->fd,jc->reader_header_ctx,SpxMsgHeaderSize , &(jc->offset));
            if(err) {
                if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                    if(SpxMsgHeaderSize != jc->offset) {
                        ev_once(jc->tc->loop,jc->fd,EV_READ,jc->waitting,jc->nio_reader,(void *) jc);
                        return;
                    }
                } else {
                    jc->err = err;
                    SpxLogFmt2(jc->log,SpxLogError,err,\
                            "read header from client:%s is fail."
                            "header length:%d,real recv length:%d."
                            "then push job to pool.",
                            jc->client_ip,SpxMsgHeaderSize,jc->offset);
                    spx_job_context_clear(jc);
                    return;
                }
            }

            spx_msg_seek(jc->reader_header_ctx,0,SpxMsgSeekSet);
            struct spx_msg_header *header = spx_msg_to_header(jc->reader_header_ctx,&err);
            if(NULL == header){
                jc->err = err;
                SpxLogFmt2(jc->log,SpxLogError,err,
                        "convert msg-ctx to header is fail."
                        "client ip:%s.",
                        jc->client_ip);
                spx_job_context_clear(jc);
                return;
            }
            jc->reader_header = header;
            if(header->bodylen < header->offset){
                jc->err = EINVAL;
                SpxLogFmt1(jc->log,SpxLogError,
                        "body len is 0 and no need read."
                        "client:%s."
                        "proto:%d,version:%d.",
                        jc->client_ip,
                        jc->reader_header->protocol,
                        jc->reader_header->version);
                spx_job_context_clear(jc);
                return;
            }

            if(NULL != jc->verifyHeaderHandler) {
                // if validator return false,then the function must push jc to pool
                if(!jc->verifyHeaderHandler(jc)){
                    return;
                }
            }

            if(NULL != jc->beforeRecviveBodyHandler){
                // if body_process_before return false,then the function must push jc to pool
                if(!jc->beforeRecviveBodyHandler(jc)){
                    return;
                }
            }
            jc->lifecycle = SpxNioLifeCycleBody;
            jc->offset = 0;
        }

        if(SpxNioLifeCycleBody == jc->lifecycle) {
            struct spx_msg_header *header = jc->reader_header;
            if(NULL == header){
                jc->err = ENXIO;
                SpxLogFmt1(jc->log,SpxLogError,
                        "reader header is null."
                        "client:%s.",
                        jc->client_ip);
                spx_job_context_clear(jc);
                return;
            }

            //only have header,the request is also dealed
            if(0 != header->offset || 0 != header->bodylen) {
                if(jc->is_lazy_recv){
                    if(NULL == jc->reader_body_ctx) {
                        struct spx_msg *ctx = spx_msg_new(header->offset,&(jc->err));
                        if(NULL == ctx){
                            SpxLogFmt2(jc->log,SpxLogError,jc->err,
                                    "alloc body buffer is fail."
                                    "client:%s."
                                    "proto:%d,version:%d.",
                                    jc->client_ip,
                                    jc->reader_header->protocol,
                                    jc->reader_header->version);
                            spx_job_context_clear(jc);
                            return;
                        }
                        jc->reader_body_ctx = ctx;
                    }

                    err = spx_read_msg_ack(jc->fd,jc->reader_body_ctx,jc->reader_header->offset , &(jc->offset));
                    if(err) {
                        if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                            //                        jc->offset += len;
                            if(header->offset != jc->offset) {
                                ev_once(jc->tc->loop,jc->fd,EV_READ,jc->waitting,jc->nio_reader,(void *) jc);
                                return;
                            }
                        } else {
                            jc->err = err;
                            SpxLogFmt2(jc->log,SpxLogError,err,\
                                    "read header from client:%s is fail."
                                    "body offset length:%d,real recv length:%d."
                                    "then push job to pool.",
                                    jc->client_ip,header->offset,jc->offset);
                            spx_job_context_clear(jc);
                            return;
                        }
                    }
                } else {
                    if(NULL == jc->reader_body_ctx) {
                        struct spx_msg *ctx = spx_msg_new(header->bodylen,&(jc->err));
                        if(NULL == ctx){
                            SpxLogFmt2(jc->log,SpxLogError,jc->err,
                                    "alloc body buffer is fail.client:%s.",
                                    "proto:%d,version:%d.",
                                    jc->client_ip,
                                    jc->reader_header->protocol,
                                    jc->reader_header->version);
                            spx_job_context_clear(jc);
                            return;
                        }
                        jc->reader_body_ctx = ctx;
                    }

                    err = spx_read_msg_ack(jc->fd,jc->reader_body_ctx,header->bodylen ,&(jc->offset));
                    if(err) {
                        if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                            //                        jc->offset += len;
                            if(header->bodylen != jc->offset) {
                                ev_once(jc->tc->loop,jc->fd,EV_READ,jc->waitting,jc->nio_reader,(void *) jc);
                                return;
                            }
                        } else {
                            jc->err = err;
                            SpxLogFmt2(jc->log,SpxLogError,err,\
                                    "read header from client:%s is fail."
                                    "body length:%d,real recv length:%d."
                                    "then push job to pool.",
                                    jc->client_ip,header->bodylen,jc->offset);
                            spx_job_context_clear(jc);
                            return;
                        }
                    }
                }
            }
            if(NULL != jc->disposeReaderHandler){
                jc->disposeReaderHandler(jc);
            }
        }
    }
}/*}}}*/

void spx_nio_writer_with_timeout_fast(int revents,void *arg){/*{{{*/
    struct spx_job_context *jc = (struct spx_job_context *) arg;
    if(NULL == jc) return;

    if (EV_ERROR & revents) {
        SpxLogFmt1(jc->log,SpxLogError,
                "waitting writing to client:%s is fail,"
                "and close the client and push job-context to pool.",
                jc->client_ip);
        spx_job_context_clear(jc);
        return;
    }

    if(EV_TIMEOUT & revents) {
        if(jc->trytimes <= (jc->curr_trytimes++)){
            SpxLogFmt1(jc->log,SpxLogError,
                    "waitting writeting to client:%s to max times:%d."
                    "then close the client and push job-context to pool.",
                    jc->client_ip,jc->curr_trytimes);
            spx_job_context_clear(jc);
            return;
        } else {
            ev_once(jc->tc->loop,jc->fd,EV_WRITE,jc->waitting,jc->nio_writer,(void *) jc);
            SpxLogFmt1(jc->log,SpxLogWarn,
                    "re-try waitting writing to client:%s again."
                    "current try-times is :%d.",
                    jc->client_ip,jc->curr_trytimes);
            return;
        }
    }

    err_t err = 0;
    if(EV_WRITE & revents) {
        if(SpxNioLifeCycleHeader == jc->lifecycle){
            if(NULL == jc->writer_header_ctx) {
                struct spx_msg *ctx = spx_header_to_msg(jc->writer_header,SpxMsgHeaderSize,&err);
                if(NULL == ctx){
                    jc->err = err;
                    SpxLog2(jc->log,SpxLogError,err,\
                            "convert writer header to msg ctx is fail."\
                            "and forced push jc to pool.");
                    spx_job_context_clear(jc);
                    return;
                }
                jc->writer_header_ctx = ctx;
            }

            err = spx_write_msg_ack(jc->fd,jc->writer_header_ctx,&(jc->offset));
            if(err) {
                if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                    if(SpxMsgHeaderSize != jc->offset) {
                        ev_once(jc->tc->loop,jc->fd,EV_WRITE,jc->waitting,jc->nio_writer,(void *) jc);
                        return;
                    }
                } else {
                    jc->err = err;
                    SpxLogFmt2(jc->log,SpxLogError,jc->err,
                            "write  header to client :%s is fail.len:%d."
                            "proto:%d,version:%d."
                            "and forced push jc to pool.",
                            jc->client_ip,jc->offset,
                            jc->writer_header->protocol,
                            jc->writer_header->version);
                    spx_job_context_clear(jc);
                    return;
                }
            }
            jc->offset = 0;
            jc->lifecycle = SpxNioLifeCycleBody;
        }

        if(SpxNioLifeCycleBody == jc->lifecycle) {
            if(NULL != jc->writer_body_ctx ) {//no metedata
                size_t len = 0 == jc->writer_header->offset ? jc->writer_header->bodylen : jc->writer_header->offset;
                err = spx_write_msg_ack(jc->fd,jc->writer_body_ctx,&(jc->offset));
                if(err) {
                    if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                        if(len != jc->offset) {
                            ev_once(jc->tc->loop,jc->fd,EV_WRITE,jc->waitting,jc->nio_writer,(void *) jc);
                            return;
                        }
                    } else {
                        jc->err = err;
                        SpxLogFmt2(jc->log,SpxLogError,jc->err,\
                                "write  body or prebody to client :%s is fail.len:%d."
                                "proto:%d,version:%d."
                                "and forced push jc to pool.",
                                jc->client_ip,len,
                                jc->writer_header->protocol,
                                jc->writer_header->version);
                        spx_job_context_clear(jc);
                        return;
                    }
                }
            }
            jc->lifecycle = SpxNioLifeCycleFile;
            jc->offset = 0;
        }

        if(SpxNioLifeCycleFile == jc->lifecycle){
            if(jc->is_sendfile){
                err = spx_sendfile_ack(jc->fd,jc->sendfile_fd,\
                        jc->sendfile_begin + jc->offset,jc->sendfile_size - jc->offset,&(jc->offset));
                if(err) {
                    if(EAGAIN == err || EWOULDBLOCK == err || EINTR == err) {
                        if(jc->sendfile_size != jc->offset) {
                            ev_once(jc->tc->loop,jc->fd,EV_WRITE,jc->waitting,jc->nio_writer,(void *) jc);
                            return;
                        }
                    } else {
                        jc->err = err;
                        SpxLogFmt2(jc->log,SpxLogError,jc->err,
                                "sendfile to client :%s is fail.len:%d."
                                "proto:%d,version:%d."
                                "and forced push jc to pool.",
                                jc->client_ip,jc->offset,
                                jc->writer_header->protocol,
                                jc->writer_header->version);
                        spx_job_context_clear(jc);
                        return;
                    }
                }
            }
            jc->lifecycle = SpxNioLifeCycleNormal;
        }

        if(NULL != jc->disposeWriterHandler){
            jc->disposeWriterHandler(jc);
        }
        return;
    }
}/*}}}*/


