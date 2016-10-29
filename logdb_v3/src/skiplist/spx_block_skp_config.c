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
#include "logdb_map.h"

#define SpxSkpIndexConf "./skiplist/config/index.config" 

struct logdb_map* g_idx_task_map = NULL;

static void free_queue(void* q);

struct spx_block_skp_index* spx_block_skp_load_index()/*{{{*/
{
    if (g_spx_block_skp_idx_head.next_idx == NULL){
        FILE *fp = fopen(SpxSkpIndexConf, "r");
        if (NULL == fp){
            printf("open %s failed\n", SpxSkpIndexConf);
            return NULL;
        }

        char line[100];
        struct spx_block_skp_idx* tmp_idx = &g_spx_block_skp_idx_head;
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

int spx_block_skp_count_config_index(){/*{{{*/
    int cnt = 0;
    struct spx_block_skp_index *tmp_idx = spx_block_skp_load_index();
    while (tmp_idx->next_idx){
        cnt++;
        tmp_idx = tmp_idx->next_idx;
    }

    return cnt;
}/*}}}*/

static void free_queue(void* q){
    struct logdb_queue* queue = (struct logdb_queue*)q;
    logdb_queue_destroy(&queue);
}

int spx_block_skp_task_map_init(){
    if (NULL == g_idx_task_map){
        g_idx_task_map = logdb_map_new(cmp_str, NULL, free_queue);
        if (NULL == g_idx_task_map){
            printf("g_idx_task_map is NULL in spx_block_skp_task_map_init\n");
            return -1;
        }
    }

    struct spx_block_skp_index* idx_lst = spx_block_skp_load_index();
    if (NULL == idx_lst){
        printf("idx_lst in spx_block_skp_task_map_init is NULL\n");
        return -1;
    }

    struct spx_block_skp_index* tmp_idx = idx_lst->next_idx;
    while (tmp_idx != NULL){
        struct logdb_queue* queue = logdb_queue_new(tmp_idx->idx);
        if (NULL == queue){
            printf("queue is NULL in spx_block_skp_task_map_init for index:%s\n", tmp_idx->idx);
            continue;
        }
        logdb_map_insert(g_idx_task_map, tmp_idx->idx, queue);
    }

    return 0;
}

struct logdb_queue* spx_block_skp_task_queue_dispatcher(char* key){
    if (NULL == key){
        printf("key is NULL in spx_block_skp_task_queue_dispatcher\n");
        return NULL;
    }
    struct logdb_queue* task_queue = logdb_map_get(g_idx_task_map, key);

    return task_queue;
}

