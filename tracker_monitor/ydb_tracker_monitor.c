/*************************************************************************
    > File Name: ydb_tracker_monitor.c
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

#include "ydb_tracker_configurtion.h"
#include "ydb_tracker_heartbeat.h"
#include "ydb_tracker_monitor.h"

#define ISspace(x) isspace((int)(x))
#define SERVER_STRING "Server: DFSHTTPD/0.1.0\r\n"
#define JSON_SIZE 1024

struct mainsocket_thread_arg{
    SpxLogDelegate *log;
    struct ydb_tracker_configurtion *c;
};

struct socket_log{
    SpxLogDelegate *log;
    int client;
};

struct StorageList{
    struct ydb_remote_storage *storage;
    struct StorageList *next_storage;
};

char storage_field[12][100]={
    "this_startup_time",
    "first_startup_time",
    "last_heartbeat",
    "disksize",
    "freesize",
    "status",
    "machineid",
    "groupname",
    "syncgroup",
    "ip",
    "port",
    "readonly"
};

spx_private ev_io monitor_watcher;
spx_private struct ev_loop *main_socket_loop = NULL;
spx_private struct ydb_remote_storage *curr_storage = NULL;

pthread_t ydb_tracker_monitor_thread_new(SpxLogDelegate *log, struct ydb_tracker_configurtion *c, err_t *err){
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    size_t ostack_size =0;
    pthread_attr_getstacksize(&attr, &ostack_size);
    if ( (ostack_size != c->stacksize) && (0 != (*err = pthread_attr_setstacksize(&attr, c->stacksize)))){
                    return 0;
    }

    struct mainsocket_thread_arg *arg = (struct mainsocket_thread_arg *) spx_alloc_alone(sizeof(*arg), err);
    if(NULL == arg){
        pthread_attr_destroy(&attr);
        return 0;
    }

    arg->log = log;
    arg->c = c;

    pthread_t tid = 0;
    if( 0 != (*err = pthread_create(&tid, &attr, ydb_tracker_monitor_mainsocket_create, arg))){
        pthread_attr_destroy(&attr);
        SpxFree(arg);
        return 0;
    }

    pthread_attr_destroy(&attr);
    return tid;
}

spx_private void * ydb_tracker_monitor_mainsocket_create(void *arg){
    struct mainsocket_thread_arg *mainsocket_arg = (struct mainsocket_thread_arg *) arg;
    SpxLogDelegate *log = mainsocket_arg->log;
    struct ydb_tracker_configurtion *c = mainsocket_arg->c;
    SpxFree(mainsocket_arg);
    err_t err = 0;
    
    main_socket_loop = ev_loop_new(0);
    if(NULL == main_socket_loop){
        SpxLog2(log, SpxLogError, err, "create main socket loop is fail.");
        return NULL;
    }

    int mainsocket = spx_socket_new(&err);
    if(0 == mainsocket){
        SpxLog2(log, SpxLogError, err, "create main socket is fail.");
        return NULL;
    }

    if( 0 != (err = spx_set_nb(mainsocket))){
        SpxLog2(log, SpxLogError, err, "set main socket nonblock is fail.");
    }

    if( 0 != (err = spx_socket_start(mainsocket, c->ip, 8989,\
                                     true, c->timeout,\
                                     3, c->timeout,\
                                     false, 0,\
                                     true,\
                                     true, c->timeout,
                                     1024))){
        SpxLog2(log, SpxLogError, err, "start main socket is fail.");
        goto r1;
}

    SpxLogFmt1(log, SpxLogMark,
               "main socket fd: %d"
               "and accepting...",
               mainsocket);
    ydb_tracker_monitor_socket_accept_nb(c->log, main_socket_loop, mainsocket);
r1:
    SpxClose(mainsocket);
    return NULL;
}

spx_private void ydb_tracker_monitor_socket_accept_nb(SpxLogDelegate *log, struct ev_loop *loop, int fd){
    SpxZero(monitor_watcher);
    ev_io_init(&monitor_watcher, ydb_tracker_monitor_socket_main_reciver, fd, EV_READ);
    monitor_watcher.data = log;
    ev_io_start(loop, &(monitor_watcher));
    ev_run(loop, 0);
}

spx_private void ydb_tracker_monitor_socket_main_reciver(struct ev_loop *loop, ev_io *watcher, int revents){
    ev_io_stop(loop, watcher);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    SpxLogDelegate *log = (SpxLogDelegate *) watcher->data;
    err_t err = 0;
    struct socket_log *sl = (struct socket_log *) spx_alloc_alone(sizeof(*sl), &err);
    sl->log = log;

    while(true){
        struct sockaddr_in client_addr;
        pthread_t tid = 0;
        unsigned int socket_len = 0;
        int client_sock = 0;
        socket_len = sizeof(struct sockaddr_in);
        client_sock = accept(watcher->fd, (struct sockaddr *) &client_addr, &socket_len);

        spx_tcp_nodelay(client_sock, true);

        if (0 > client_sock){
            //if( EWOULDBLOCK == errno || EAGAIN == ERRNO){
            //    break;
            //}
            break;
        }

        if( 0  == client_sock){
            break;
        }

        sl->client = client_sock;
        if (0 != (err = pthread_create(&tid, &attr, ydb_tracker_monitor_socket_main_response, sl))){
            SpxLog2(log, SpxLogError, err, "create ydb_tracker_monitor_socket_main_response faild.");
        }
    }

    ev_io_start(loop, watcher);
}

spx_private err_t getRequest(int sock, char *buf, size_t size, size_t *len){
    SpxErrReset;
    *len = 0;
    err_t err = 0;
    i64_t rc = 0;
    int flags = fcntl(sock, F_GETFL);
    flags |= O_NONBLOCK;

    if( -1 == fcntl(sock, F_SETFL, flags)){
        perror("fcntl");
    }

    while(*len < size){
        rc = read(sock ,((char *) buf) + *len,size - *len);
        if(0 > rc){
//            if(EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno){
//                SpxErrReset;
//                continue;
//            }
            err = errno;
            break;
        }else if(0 == rc){
            break;
        }else {
            *len += rc;
        }
    }
    if(0 != err) return err;
    if(size != *len) return EIO;
    return 0;
}/*}}}*/


spx_private void * ydb_tracker_monitor_socket_main_response(void * arg){
    char buf[1024];
    char method[255];
    char url[1024];
//   char path[1024];
    char *param[100][2];
    size_t i = 0, j = 0;
    size_t len = 0;
    err_t err = 0;
    struct socket_log * sl = (struct socket_log *) arg;
    int client_sock = sl->client;
    SpxLogDelegate * log = sl->log;
    SpxFree(arg);

//    if( 0 != (err = spx_read(client_sock, (byte_t *)buf, sizeof(buf), &len))){
//        SpxLog2(log, SpxLogError, err, "read socket:%d failed");
//        return NULL;
//    }

    getRequest(client_sock, buf, sizeof(buf), &len);

    printf("%s\n",buf);

    while(!ISspace(buf[j]) && (i < sizeof(method) -1)){
        method[i] = buf[j];
        i++; j++;
    }

    method[i] = '\0';

    if (strcasecmp(method, "GET") && strcasecmp(method, "POST")){
        unimplemented(client_sock);
        return NULL;
    }

    i = 0;
    while(ISspace(buf[j]) && (j < sizeof(buf)))
        j++;
    while (!ISspace(buf[j]) && (i < sizeof(url) -1) && (j < sizeof(buf))){
        url[i] = buf[j];
        i++;
        j++;
    }

    url[i] = '\0';

    size_t url_idx = 0, param_count = 0; 
    while((url[url_idx] != '?') && (url[url_idx] != '\0')){
//        path[url_idx] = url[url_idx];
        url_idx++;
    }


    if(url_idx < strlen(url) && url[url_idx] == '?')
        url_idx++;
    while(url_idx < strlen(url) && url[url_idx] != '\0'){
        char tmp[100];
        size_t tmp_len = 0;

        while(url[url_idx] != '&' && tmp_len < sizeof(tmp)){
             tmp[tmp_len++] = url[url_idx++];
        }

        size_t k1 = 0, k2 = 0;
        char tmpKey[100];
        char tmpVal[100];

        while(tmp[k1] != '=' && k2 < tmp_len){
            tmpKey[k1++] = tmp[k2++];
        }

        param[param_count][0] = (char*)malloc(sizeof(char)*k1);
        strncpy(param[param_count][0], tmpKey, k1);
        k2++;
        k1 = 0;
        while(k2 < tmp_len){
            tmpVal[k1++] = tmp[k2++];
        }
        param[param_count][1] = (char*)malloc(sizeof(char)*k1);
        strncpy(param[param_count][1], tmpVal, k2);
        param_count++;
        url_idx++;
    }

    if(strcasecmp(method, "GET") == 0){
        struct spx_job_context *jc = spx_job_pool_pop(g_spx_job_pool, &err);
        if(NULL == jc){
            SpxClose(client_sock);
            SpxLog1(log, SpxLogError, \
                    "pop nio context is fail.");
            return NULL;
        }

        SendStorageJSON(client_sock, param[0][1], jc); 
    }

    if(strcasecmp(method, "POST") == 0){
        unimplemented(client_sock);
        return NULL;
    }



    SpxClose(client_sock);
    for(i = 0; i < param_count; i++){
        free(param[i][0]);
        free(param[i][1]);
     }

    return NULL;
}

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
/**********************************************************************/
spx_private void bad_request(int client)
{
 char head[1024];
 char tmp[1024];
 char body[1024] = {"<HTML><BODY><P>Your browser sent a bad request, such as a POST without a Content-Length.</HTML></BODY>\r\n"};
 int content_length = strlen(body);

 sprintf(tmp, "HTTP/1.1 400 BAD REQUEST\r\n");
 strncpy(head, tmp, strlen(tmp));
 sprintf(tmp, "Content-type: text/html\r\n");
 strncat(head, tmp, strlen(tmp));
 sprintf(tmp, "Content-Length: %d\r\n", content_length);
 strncat(head, tmp, strlen(tmp));
 sprintf(tmp, "\r\n");
 strncat(head, tmp, strlen(tmp));

 send(client, head, strlen(head), 0);
 send(client, body, strlen(body), 0);
}

/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
 *             the name of the file */
/**********************************************************************/
spx_private void headers(int client, int content_length)
{
 char head[1024];
 char tmp[1024];

 sprintf(tmp, "HTTP/1.1 200 OK\r\n");
 strncpy(head, tmp,  strlen(tmp));
 sprintf(tmp, SERVER_STRING);
 strncat(head, tmp, strlen(tmp));
 sprintf(tmp, "Content-Type: text/html;charset=UTF-8\r\n");
 strncat(head, tmp, strlen(tmp));
 sprintf(tmp, "Content-Length: %d\r\n", content_length);
 strncat(head, tmp, strlen(tmp));
 sprintf(tmp, "\r\n");
 strncat(head, tmp, strlen(tmp));
 
 send(client, head, strlen(head), 0);
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
/**********************************************************************/
spx_private void unimplemented(int client)
{
 char head[1024];
 char body[1024];
 char buf[4096];
 char tmp[1024];
 int content_length = 0;

 sprintf(tmp, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
 strncpy(body, tmp, strlen(tmp));
 sprintf(tmp, "</TITLE></HEAD>\r\n");
 strncat(body, tmp, strlen(tmp));
 sprintf(tmp, "<BODY><P>HTTP request method not supported.\r\n");
 strncat(body, tmp, strlen(tmp));
 sprintf(tmp, "</BODY></HTML>\r\n");
 strncat(body, tmp, strlen(tmp));

 strncpy(buf, body, strlen(body));

 content_length = strlen(body);

// sprintf(tmp, "HTTP/1.0 501 Method Not Implemented\r\n");
// strncpy(head, tmp, strlen(tmp));
// sprintf(tmp, SERVER_STRING);
// strncat(head, tmp, strlen(tmp));
// sprintf(tmp, "Content-Type: text/html\r\n");
// strncat(head, tmp, strlen(tmp));
// sprintf(tmp, "Content-Length: %d\r\n",content_length);
// strncat(head, tmp, strlen(tmp));
// sprintf(tmp, "\r\n");
// strncat(head, tmp, strlen(tmp));
// 
//
// send(client, head, strlen(head), 0);
 headers(client, content_length);
 send(client, body, strlen(body), 0);
}

/**********************************************************************/
/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
/**********************************************************************/
spx_private struct StorageList * VisitStorageMap(struct spx_map *map, struct spx_job_context *jc){
    if(NULL == map){
        jc->err = ENOENT;
        return NULL;
    }

    struct spx_map_iter *curr_iter = spx_map_iter_new(map, &(jc->err));
    if(NULL == curr_iter){
        return NULL;
    }

    struct StorageList * storage_list_head = (struct StorageList*)calloc(1, sizeof(struct StorageList));
    struct StorageList* storage_list_cur = storage_list_head;
    struct spx_map_node *n = NULL;

    while((n = spx_map_iter_next(curr_iter, &(jc->err))) != NULL){
        struct ydb_remote_storage *storage = (struct ydb_remote_storage *) n->v;
        if(NULL == storage){
            continue;
        }
    
    struct StorageList *tmp_storage = (struct StorageList*) calloc(1, sizeof(struct StorageList));
    tmp_storage->storage = storage;
    storage_list_cur->next_storage = tmp_storage;
    storage_list_cur = tmp_storage;
   }

    spx_map_iter_free(&curr_iter);

    return storage_list_head;
}

spx_private struct StorageList *ydb_tracker_visit_storage(
        string_t groupname, struct spx_job_context *jc){
    if(NULL == ydb_remote_storages){
        jc->err = ENOENT;
        return NULL;
    }

    struct ydb_tracker_configurtion *c = (struct ydb_tracker_configurtion *) jc->config;
            
    struct spx_map *group_map = ydb_remote_storages; 
    if(NULL != groupname){
        struct spx_map *storage_map = spx_map_get(group_map, groupname, spx_string_rlen(groupname), NULL);
        spx_job_pool_push(g_spx_job_pool, jc);

        return VisitStorageMap(storage_map, jc);
    } else{
        struct spx_map_iter *group_curr_iter = spx_map_iter_new(group_map, &(jc->err));
        if(NULL == group_curr_iter){
            return NULL;
        }

        struct spx_map_node *group_node = NULL;
        struct StorageList * storage_list_head = (struct StorageList*)calloc(1, sizeof(struct StorageList));
        struct StorageList * storage_list_cur = storage_list_head;

        while((group_node = spx_map_iter_next(group_curr_iter, &(jc->err))) != NULL){
            struct spx_map *storage_map  = (struct spx_map *) group_node->v;

            if( NULL == storage_map){
                jc->err = ENOENT;
                return NULL;
            }

            struct spx_map_iter * storage_curr_iter =spx_map_iter_new(storage_map, &(jc->err));
            if(NULL == storage_curr_iter){
                return NULL;
            }

            struct spx_map_node * storage_node = NULL;
            while((storage_node = spx_map_iter_next(storage_curr_iter, &(jc->err))) != NULL){
                struct ydb_remote_storage * storage = (struct ydb_remote_storage *) storage_node->v;
                if(NULL == storage){
                    continue;
                }

                struct StorageList *tmp_storage = (struct StorageList *) calloc(1, sizeof(struct StorageList));
                tmp_storage->storage = storage;
                storage_list_cur->next_storage = tmp_storage;
                storage_list_cur = tmp_storage;
            }

            spx_map_iter_free(&storage_curr_iter);
            
        }

        spx_map_iter_free(&group_curr_iter);

        return storage_list_head;
    }
}

spx_private void SendStorageJSON(int client, string_t groupname , struct spx_job_context *jc){
    struct StorageList *list = ydb_tracker_visit_storage(groupname, jc);
    if(list == NULL){
        bad_request(client);
        return;
    }

    char buf[8192];
    char tmp[1024];
    sprintf(tmp, "storages : [\r\n");
    strncpy(buf, tmp, strlen(tmp));

    struct StorageList *cur_storage = list;
    while(cur_storage->next_storage != NULL){
        cur_storage = cur_storage->next_storage;
        if(cur_storage == NULL)
            break;
        char * storage_json = Storage2JSON(cur_storage->storage);
        sprintf(tmp, "%s\r\n", storage_json);
        strncat(buf, tmp, strlen(tmp));
        free(storage_json);
    }

    sprintf(tmp, "]\r\n");
    strncat(buf, tmp, strlen(tmp));

    headers(client, strlen(buf));
    send(client, buf, strlen(buf), 0); 

    FreeStorageList(list);
}

spx_private void FreeStorageList(struct StorageList *list){
    struct StorageList * cur_storage = list;

    while(cur_storage->next_storage != NULL){
        struct StorageList *tmp_storage = cur_storage;
        cur_storage = cur_storage->next_storage;
        free(tmp_storage);
    }

    free(cur_storage);
}

spx_private char * Storage2JSON(struct ydb_remote_storage * storage){
    char * storage_json = (char *) calloc(1, sizeof(char) * JSON_SIZE); 
    strncpy(storage_json, "{", 1);
    char this_startup_time[100];
    char first_startup_time[100];
    char last_heartbeat[100];
    char disksize[100];
    char freesize[100];
    char status[100];
    char machineid[100];
    char groupname[100];
    char syncgroup[100];
    char ip[100];
    char port[100];
    char readonly[100];
    
    sprintf(this_startup_time, "this_startup_time :%ld, ", storage->this_startup_time);
    strncat(storage_json, this_startup_time, strlen(this_startup_time));

    sprintf(first_startup_time,"first_startup_time : %ld, ", storage->first_startup_time);
    strncat(storage_json, first_startup_time, strlen(first_startup_time));

    sprintf(last_heartbeat, "last_heartbeat : %ld, ", storage->last_heartbeat);
    strncat(storage_json, last_heartbeat, strlen(last_heartbeat));

    sprintf(disksize, "disksize : %ld, ", storage->disksize);
    strncat(storage_json, disksize, strlen(disksize));

    sprintf(freesize, "freesize : %ld, ", storage->freesize);
    strncat(storage_json, freesize, strlen(freesize));

    sprintf(status, "status : %d, ", storage->status);
    strncat(storage_json, status, strlen(status));

    sprintf(machineid, "machineid : %s, ", storage->machineid);
    //printf("machineid : %s\n", storage->machineid);
    strncat(storage_json, machineid, strlen(machineid));

    sprintf(groupname, "groupname : %s, ", storage->groupname);
    //printf("groupname : %s\n ", storage->groupname);
    strncat(storage_json, groupname, strlen(groupname));

    sprintf(syncgroup, "syncgroup : %s, ", storage->syncgroup);
    strncat(storage_json, syncgroup, strlen(syncgroup));

    sprintf(ip, "ip : %s, ", storage->ip);
    strncat(storage_json, ip, strlen(syncgroup));

    sprintf(port, "port : %d, ", storage->port);
    strncat(storage_json, port, strlen(port));

    sprintf(readonly, "readonly : %d", storage->readonly);
    strncat(storage_json, readonly, strlen(readonly));

    strncat(storage_json, "}", 1);

    return storage_json;
}
