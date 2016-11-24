/*************************************************************************
    > File Name: spx_block_skp_config.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Fri 28 Oct 2016 04:14:08 PM CST
************************************************************************/
#include <string.h>
#include <stdio.h>
#include "spx_block_skp_config.h"
#include "spx_block_skp_common.h"
#include "logdb_skp_common.h"
#include "spx_block_skp_serial.h"
#include "logdb_map.h"
#include "spx_block_skp.h"

#define SpxSkpIndexConf "../skiplist/config/index.config" 
#define TaskPoolSize 10 

struct logdb_map* g_task_queue = NULL;
struct logdb_map* g_skp_pool = NULL;
struct logdb_queue* g_task_pool = NULL;

static void free_task_queue(void* q);
static void free_block_skp(void* skp);
static void free_md(void* mdp);

static void free_task_queue(void* q){/*{{{*/
    struct logdb_queue* queue = (struct logdb_queue*)q;
    logdb_queue_destroy(&queue);
}/*}}}*/

static void free_block_skp(void* skp){/*{{{*/
    struct spx_block_skp* block_skp = (struct spx_block_skp*) skp;
    spx_block_skp_destroy(&block_skp);
}/*}}}*/

static void free_md(void* mdp){/*{{{*/
    if (NULL == mdp){
        printf("mdp is NULL in free_md\n");
    }

    struct spx_skp_serial_metadata* md = (struct spx_skp_serial_metadata*)mdp;
    spx_skp_serial_md_free(md);
}/*}}}*/


struct spx_block_skp_index* spx_block_skp_config_load_index()/*{{{*/
{
    if (g_spx_block_skp_idx_head.next_idx == NULL){
        FILE *fp = fopen(SpxSkpIndexConf, "r");
        if (NULL == fp){
            printf("open %s failed\n", SpxSkpIndexConf);
            return NULL;
        }

        char line[100];
        struct spx_block_skp_index* tmp_idx = &g_spx_block_skp_idx_head;
        while(fgets(line, sizeof(line), fp)){
            if('#' != *line && '\n' != *line){
                int i = 0;
                for(i = 0; i < (int)strlen(line); i++){
                    if( *(line + i) == ':')
                    break;
                }

                struct spx_block_skp_index* new_idx = (struct spx_block_skp_index *) malloc(sizeof(*new_idx));           
                new_idx->next_idx = NULL;
                new_idx->idx= (char *) malloc(sizeof(char) * (i + 1));
                strncpy(new_idx->idx, line, i);
                *(new_idx->idx+ i) = '\0';
                if (!strcmp((line + i + 1), "string\n")){
                    new_idx->type = 0;  
                } else if(!strcmp((line + i + 1), "int\n")){
                    new_idx->type = 1;
                } else if(!strcmp((line + i + 1), "long\n")){
                    new_idx->type = 2;
                } else{
                    new_idx->type = -1;
                }

                tmp_idx->next_idx = new_idx; 
                tmp_idx = new_idx;
            }
        }
    }

    return &g_spx_block_skp_idx_head;
}/*}}}*/

int spx_block_skp_config_count_index(){/*{{{*/
    int cnt = 0;
    struct spx_block_skp_index *tmp_idx = spx_block_skp_config_load_index();
    while (tmp_idx->next_idx){
        cnt++;
        tmp_idx = tmp_idx->next_idx;
    }

    return cnt;
}/*}}}*/


int spx_block_skp_config_task_pool_init(){/*{{{*/
    g_task_pool = logdb_queue_new("task_pool");
    if (NULL == g_task_pool){
        printf("g_task_pool is NULL in spx_block_skp_task_pool_init\n");
        return -1;
    }

    int i = 0;
    for (i = 0; i < TaskPoolSize; i++){
        struct spx_block_skp_task* task = (struct spx_block_skp_task*) malloc(sizeof(*task));
        task->key = NULL;
        task->value = NULL;
        task->skp = NULL; 
        logdb_queue_push(g_task_pool, task);
    }

    return 0;
}/*}}}*/

struct spx_block_skp_task* spx_block_skp_config_task_pool_pop(){/*{{{*/
    struct spx_block_skp_task* task = logdb_queue_pop(g_task_pool);
    while (NULL == task)
        task = logdb_queue_pop(g_task_pool);
    return task;
}/*}}}*/

int spx_block_skp_config_task_pool_push(struct spx_block_skp_task* task){/*{{{*/
    if (NULL == task){
        printf("task is NULL in spx_block_skp_task_pool_push\n");
        return -1;
    }

    if (NULL == task->skp){
        printf("skp in task is NULL and key, value will not be free\n");
        return -1;
    }

    if (task->key != NULL)
        task->skp->free_key(task->key);
    if (task->value != NULL)
        task->skp->free_value(task->value);

    task->key = NULL;
    task->value = NULL;
    task->skp = NULL;

    logdb_queue_push(g_task_pool, task);

    return g_task_pool->size;
}/*}}}*/

int spx_block_skp_config_task_queue_init(){/*{{{*/
    if (NULL == g_task_queue){
        g_task_queue = logdb_map_new(cmp_str, NULL, free_task_queue);
        if (NULL == g_task_queue){
            printf("g_task_queue is NULL in spx_block_skp_task_queue_init\n");
            return -1;
        }
    }

    struct spx_block_skp_index* idx_lst = spx_block_skp_config_load_index();
    if (NULL == idx_lst){
        printf("idx_lst in spx_block_skp_task_queue_init is NULL\n");
        return -1;
    }

    struct spx_block_skp_index* tmp_idx = idx_lst->next_idx;
    while (tmp_idx != NULL){
        struct logdb_queue* queue = logdb_queue_new(tmp_idx->idx);
        if (NULL == queue){
            printf("queue is NULL in spx_block_skp_task_queue_init for index:%s\n", tmp_idx->idx);
            continue;
        }
        logdb_map_insert(g_task_queue, tmp_idx->idx, queue);
        tmp_idx = tmp_idx->next_idx;
    }

    return 0;
}/*}}}*/

struct logdb_queue* spx_block_skp_config_task_queue_dispatcher(char* key){/*{{{*/
    if (NULL == key){
        printf("key is NULL in spx_block_skp_task_queue_dispatcher\n");
        return NULL;
    }
    struct logdb_queue* task_queue = logdb_map_get(g_task_queue, key);

    return task_queue;
}/*}}}*/


int spx_block_skp_config_pool_init(){/*{{{*/
    if (NULL == g_skp_pool){
        g_skp_pool = logdb_map_new(cmp_str, NULL, free_block_skp);
        if (NULL == g_skp_pool){
            printf("g_skp_pool is NULL in spx_block_skp_pool_init\n");
            return -1;
        }
    }

    struct spx_block_skp_index* idx_lst = spx_block_skp_config_load_index();
    if (NULL == idx_lst){
        printf("idx_lst is NULL in spx_block_skp_pool_init\n");
        return -1;
    }

    struct spx_block_skp_index* tmp_idx = idx_lst->next_idx;
    while (tmp_idx != NULL){
        SpxSkpType key_type = SKP_STR;
        SpxSkpType value_type = SKP_MD;
        CmpDelegate cmp_key = cmp_str;
        CmpDelegate cmp_value = cmp_md;
        FreeDelegate free_key = NULL;
        FreeDelegate free_value = free_md;
        switch (tmp_idx->type){
            case 0:
                key_type = SKP_STR;
                cmp_key = cmp_str;
                break;
            case 1:
                key_type = SKP_INT;
                cmp_key = cmp_int;
                break;
            case 2:
                key_type = SKP_LONG;
                cmp_key = cmp_long;
                break;
            default:
                printf("not support type %s\n", tmp_idx->idx);
                continue;
        }

        struct spx_block_skp* new_block_skp = (struct spx_block_skp*) spx_block_skp_new(key_type, value_type, cmp_key, cmp_value, tmp_idx->idx, free_key, free_value);
        if (NULL == new_block_skp){
            printf("new_block_skp is NULL in spx_block_skp_pool_init\n");
            continue;
        }
        logdb_map_insert(g_skp_pool, tmp_idx->idx, new_block_skp);
        tmp_idx = tmp_idx->next_idx;
    }
    return 0;
}/*}}}*/

struct spx_block_skp* spx_block_skp_config_pool_dispatcher(char* key){/*{{{*/
    if (NULL == key){
        printf("key is NULL in spx_block_skp_task_queue_dispatcher\n");
        return NULL;
    }
    struct spx_block_skp* block_skp = logdb_map_get(g_skp_pool, key);

    return block_skp;
}/*}}}*/
