/************************************************************************
> File Name: test.c
> Author: shuaixiang
> Mail: shuaixiang@yuewen.com
> Created Time: Mon 15 Feb 2016 07:33:18 AM UTC
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "spx_types.h"
#include "spx_string.h"
#include "spx_defs.h"
#include "spx_log.h"
#include "spx_socket.h"
#include "spx_env.h"
#include "spx_nio.h"
#include "spx_configurtion.h"
#include "spx_module.h"
#include "spx_job.h"
#include "spx_task.h"
#include "spx_notifier_module.h"
#include "spx_network_module.h"
#include "spx_task_module.h"
#include "spx_alloc.h"

#include "logdb_server.h"
#include "logdb_tracker_configuration.h"
#include "messagepack.h"
#include "skiplist.h"

#define QUERY 0
#define ADD 1
#define DELETE 2
#define UPDATE 3
#define FrzCtxPoolSize 1024
#define MsgpkTreeCtxPoolSize 1024

long server_handler(size_t req_size, char * request, char **response){/*{{{*/
   // printf("\n=====================unpacking==================\n");
    struct msgpk_object *obj = msgpk_message_unpacker((ubyte_t*)request, req_size);
    msgpk_tree_print_json(obj);
    //printf("\n====================printTree==================\n");
    //msgpk_tree_print(obj, 0);
    //printf("\n=====================request==================\n");
    //msgpk_print_hex((ubyte_t *)request, req_size);
    //printf("\n================findNode==================\n");
    printf("\n================A Request==================\n");

    if (OBJ_TYPE_EXT != obj->obj_type){
        printf("obj is not a OBJ_TYPE_EXT\n");
        msgpk_tree_free(obj);
        return -1;
    }

    if (QUERY == obj->key.int8_val){
        printf("QUERY\n");
        struct spx_skp_serial_metadata_list *md_lst = msgpk_tree_query(obj);
        struct spx_skp_serial_metadata_list_node *tmd = md_lst->head;
        msgpk_tree_free(obj);

        if (NULL == tmd){
            spx_skp_serial_metadata_list_free(md_lst);
            return -1;    
        }

        struct tree_context *ctx = msgpk_build_init(); 
        msgpk_build_tree_array_begin(ctx);
        int md_cnt = 0;
        while(tmd){
            struct spx_skp_serial_metadata *md = tmd->md;
            char unid[100];
            spx_skp_serial_gen_unid(md, unid, sizeof(unid));
            ubyte_t *data = spx_skp_serial_data_reader(md);
            msgpk_build_tree_bin(ctx, data, md->len);
            md_cnt++;
            tmd = tmd->next_md;
        }
        msgpk_build_tree_array_end(ctx, md_cnt);
        spx_skp_serial_metadata_list_free(md_lst);
        printf("\n======================response tree==================\n");
        msgpk_tree_print(ctx->root, 0);
        struct pack_buffer *pb = msgpk_message_packer(ctx->root->child);
        msgpk_print_hex(pb->buffer, pb->off);
        printf("\n----------------Response----------------\n");
        msgpk_build_free(ctx);
        *response = (char *)pb->buffer;
        return pb->off;
    } else if (ADD == obj->key.int8_val){
        printf("ADD\n");
        char * unid = msgpk_tree_add(obj, req_size, request);
        *response = unid;
        msgpk_tree_free(obj);
        if (NULL == unid)
            return -1;
        else
            return strlen(unid);
    } else if (DELETE == obj->key.int8_val){
        msgpk_tree_free(obj);
        printf("DELETE\n");
    } else if (UPDATE == obj->key.int8_val){
        msgpk_tree_free(obj);
        printf("UPDATE\n");
    } else{
        msgpk_tree_free(obj);
        printf("%d is not defined\n", obj->key.int8_val);
        return -1;
    }

    //nothing to do
    *response = NULL;
    return -1;
}/*}}}*/

int main(int argc, char **argv){/*{{{*/
    umask(0);
    SpxLogDelegate *log = spx_log;
    //    printf("%d.\n",getpid());
    if(2 != argc){
        SpxLog1(log,SpxLogError,"no the configurtion file in the argument.");
        return ENOENT;
    }

    err_t err = 0;
    string_t confname = spx_string_new(argv[1], &err);
    if(NULL == confname){
        SpxLog2(log,SpxLogError,err,
                "new the confname is fail."
                "and will exit...");
        exit(err);
    }

    struct logdb_tracker_configurtion *c = (struct logdb_tracker_configurtion *) \
    spx_configurtion_parser(log,\
                            logdb_tracker_config_before_handle,\
                            NULL,\
                            confname,\
                            logdb_tracker_config_line_parser_handle,\
                            &err);
    if(NULL == c){
        SpxLogFmt2(log,SpxLogError,err,
                   "parser the configurtion is fail.file name:%s."
                   "and will exit...",
                   confname);
        exit(err);
    }

    pthread_t tid = spx_block_skp_thread_new(log, &err);
    if (err != 0){
        SpxLog2(log, SpxLogError, err, "spx_skp_serial_block_skp_thread_new failed");
        exit(err);
    }

    struct server_config * conf = (struct server_config *) spx_alloc_alone(sizeof(*conf), &err);    
    conf->ip = c->ip;
    conf->port = c->port;
    conf->timeout = c->timeout;
    conf->context_pool_size = 1024;
    conf->log = log;
    conf->retry_times = 3;
    conf->stacksize = c->stacksize;

    if(0 != StartLogDB(server_handler, conf)){
        printf("start server failed\n");
        SpxFree(conf);
    }

    return 0;
}/*}}}*/
