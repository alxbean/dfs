/*
 * =====================================================================================
 *
 *       Filename:  spx_nio.h
 *
 *    Description:  ,e
 *
 *        Version:  1.0
 *        Created:  2014/06/09 17时43分29秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef _SPX_NIO_H_
#define _SPX_NIO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <ev.h>

#include "spx_types.h"
#include "spx_job.h"
#include "spx_task.h"

    typedef void (SpxAsyncDelegate)(struct ev_loop *loop, ev_async *w, int revents);

//err_t  spx_nio_regedit_reader(struct ev_loop *loop,int fd,struct spx_job_context *jcontext);
//err_t  spx_nio_regedit_writer(struct ev_loop *loop,int fd,struct spx_job_context *jcontext);
//err_t  spx_dio_regedit_reader(struct ev_loop *loop,int fd,ev_io *watcher,
//        SpxNioDelegate *dio_reader,void *data);
//err_t  spx_dio_regedit_writer(struct ev_loop *loop,int fd,ev_io *watcher,
//        SpxNioDelegate *dio_writer,void *data);
err_t  spx_dio_regedit_async(ev_async *w,
        SpxAsyncDelegate *reader,void *data);

/*
 * the 4 functions just support read and write network stream
 * and it is the base for you DIY-function.
 * so if you want to use these,please add the error-dealing with the fucntions
 * and you must deal the job context by yourself.
 */
//void spx_nio_reader(struct ev_loop *loop,ev_io *watcher,int revents);
//void spx_nio_writer(struct ev_loop *loop,ev_io *watcher,int revents);
//void spx_nio_writer_faster(struct ev_loop *loop,int fd,struct spx_job_context *jc);



//void spx_nio_reader_body_handler(struct ev_loop *loop,int fd,struct spx_job_context *jcontext);
//void spx_nio_writer_body_handler(struct ev_loop *loop,int fd,struct spx_job_context *jcontext);
//void spx_nio_writer_body_faster_handler(struct ev_loop *loop,int fd,struct spx_job_context *jcontext);

void spx_nio_reader_with_timeout(int revents,void *arg);
void spx_nio_writer_with_timeout(int revents,void *arg);
void spx_nio_reader_with_timeout_fast(int revents,void *arg);
void spx_nio_writer_with_timeout_fast(int revents,void *arg);



#ifdef __cplusplus
}
#endif
#endif
