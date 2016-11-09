/*************************************************************************
> File Name: struct spx_block_skp.c
> Author: shuaixiang
> Mail: shuaixiang@yuewen.com
> Created Time: Mon 07 Sep 2015 03:13:11 AM UTC
************************************************************************/
#include <ev.h>
#include <time.h>
#include "spx_block_skp.h"
#include "spx_block_skp_config.h"
#include "spx_block_skp_serial.h"
#include "spx_string.h"
#include "spx_log.h"
#include "thread_pool.h"

#define SpxSkpIndexConf "./skiplist/config/index.config" 

#define NewBlockNode(n) ((struct spx_block_skp_node *) calloc(1, sizeof(struct spx_block_skp_node) + n*sizeof(struct spx_block_skp_node*)))

//block_skp
static int spx_block_skp_node_free(struct spx_block_skp* block_skp, struct spx_block_skp_node* block_node);
static int spx_block_skp_insert(struct spx_block_skp* block_skp, void** key, void* value);

//block task
static void spx_block_skp_handler(void *arg);
static void spx_block_skp_insert_task(void *arg);


/*
 * private method
 */

//create a new node of struct spx_block_skp
struct spx_block_skp_node * spx_block_skp_node_new(int level , void * left_key, void *right_key, int64_t index){/*{{{*/
    int node_level = level + 1;
    struct spx_block_skp_node *np = NewBlockNode(node_level);
    if(np == NULL)
        return NULL;
    np->level = level;
    np->left_key = left_key;
    np->right_key = right_key;
    np->index = index;
    return np;    
}/*}}}*/ 

//free node 
static int spx_block_skp_node_free(struct spx_block_skp *block_skp, struct spx_block_skp_node *block_node){/*{{{*/
    if (NULL == block_skp){
        printf("spx_block_skp_node free block_skp is NULL\n");
        return -1;
    }

    if (NULL == block_node){
        printf("spx_skp_node free block_node is NULL\n");
        return -1;
    }


    block_skp->free_key(block_node->left_key);//do not free key pointer, if it is need, a free_method should be recall
    block_skp->free_key(block_node->right_key);//do not free key pointer, if it is need, a free_method should be recall
    free(block_node);// free skp_node

    return 0;
}/*}}}*/

/*
 * spx_block_skp operation
 */
int spx_block_skp_node_insert(struct spx_block_skp *block_skp, struct spx_block_skp_node *node){/*{{{*/
    if (NULL == block_skp){
        printf("block_skp is NULL in spx_block_skp_node_insert\n");
        return -1;
    }

    if (NULL == node){
        printf("node is NULL in spx_block_skp_node_insert\n");
        return -1;
    }

    struct spx_block_skp_node *update[SpxSkpMaxLevel];
    struct spx_block_skp_node *cur_node = NULL, *pre_node = block_skp->head;

    int i;
    //find the nodes to insert after
    for (i = block_skp->level; i >= 0; i--){
        while ((cur_node = pre_node->next_node[i]) != NULL){
            if ((cur_node->right_key != NULL) && (block_skp->cmp_key(node->left_key, cur_node->right_key) > 0))
                pre_node = cur_node; 
            else if ((cur_node->left_key != NULL) && (block_skp->cmp_key(node->right_key, cur_node->left_key) < 0)){ 
                break;//level down
            } else {
                printf("impossible key range to unserial\n");
                return -1;
            }
        }

        update[i] = pre_node;
    }
    
    if (node->level > block_skp->level){
        for (i = block_skp->level + 1; i <= node->level; i++) update[i] = block_skp->head;
            block_skp->level = node->level;
    }

    for (i = node->level; i >= 0; i--){
        node->next_node[i] = update[i]->next_node[i];
        update[i]->next_node[i] = node;
    }

    block_skp->length++;

    return 0;
}/*}}}*/

//create a new spx_block_skp
struct spx_block_skp * spx_block_skp_new(SpxSkpType key_type, SpxSkpType value_type, CmpDelegate cmp_key, CmpDelegate cmp_value, const char * block_skp_name, FreeDelegate free_key, FreeDelegate free_value){/*{{{*/
    struct spx_block_skp * block_skp = (struct spx_block_skp*) malloc(sizeof(struct spx_block_skp));
    if (block_skp == NULL){
        printf("malloc block_skp failed\n");
        return NULL;
    }

    strncpy(block_skp->name, block_skp_name, strlen(block_skp_name));
    *(block_skp->name + strlen(block_skp_name)) = '\0';

    block_skp->level = 0; 
    block_skp->cmp_key = cmp_key;
    block_skp->cmp_value = cmp_value;
    block_skp->key_type = key_type;
    block_skp->value_type = value_type;
    pthread_mutex_init(&block_skp->mutex, NULL);

    if (NULL == free_key){
        block_skp->free_key = spx_skp_default_free;
    } else {
        block_skp->free_key = free_key;
    }

    if (NULL == free_value){
        block_skp->free_value = spx_skp_default_free;
    } else {
        block_skp->free_value = free_value;
    }

    struct spx_block_skp_node * head = spx_block_skp_node_new(SpxSkpMaxLevel, NULL, NULL, -1);
    if (NULL == head){
        printf("new spx_block_skp_node failed\n");
        free(block_skp);
        return NULL;
    }

    int i;
    for (i = 0; i < SpxSkpMaxLevel; i++){
        head->next_node[i] = NULL;
    }

    block_skp->head = head;
    block_skp->length= 0;
    srand(time(0));

    return block_skp;
}/*}}}*/

//destory the struct spx_block_skp
int spx_block_skp_destroy(struct spx_block_skp ** p_block_skp){/*{{{*/
    if (NULL == p_block_skp){
        printf("p_block_skp is NULL\n");
        return -1;
    }

    if (NULL == *p_block_skp){
        printf("block_skp is NULL\n");
        return -1;
    }

    struct spx_block_skp *block_skp = *p_block_skp;
    struct spx_block_skp_node *tmp_block_skp_node = block_skp->head;
    while (tmp_block_skp_node){
        struct spx_block_skp_node * free_block_skp_node = tmp_block_skp_node;
        tmp_block_skp_node = tmp_block_skp_node->next_node[0];
        spx_block_skp_node_free(block_skp, free_block_skp_node);
    }

    free(block_skp);
    *p_block_skp = NULL;

    return 0;
}/*}}}*/

//insert a node to struct spx_block_skp
static int spx_block_skp_insert(struct spx_block_skp* block_skp, void** p_key, void* value){/*{{{*/
    if (block_skp == NULL){
        printf("block_skp is NULL\n");
        return -1;
    }

    //lock block_skp
    pthread_mutex_lock(&block_skp->mutex);
    //pthread_mutex_lock(&block_skp->mutex);//lock block_skp

    struct spx_block_skp_node *update[SpxSkpMaxLevel];
    struct spx_block_skp_node *cur_node = NULL, *pre_node = block_skp->head;
    struct spx_block_skp_node *target_node = NULL;
    int64_t off = 0;
    int64_t len = 0;
    void* key = *p_key;

    int i;
    //find the nodes to insert
    for (i = block_skp->level; i >= 0; i--){
        while ((NULL == target_node) && ((cur_node = pre_node->next_node[i]) != NULL)){
            if ((cur_node->left_key != NULL) && (block_skp->cmp_key(key, cur_node->left_key) >= 0))
                pre_node = cur_node; 
            else if ((pre_node->right_key != NULL) && (block_skp->cmp_key(key, pre_node->right_key) > 0)) 
                break;//level down
            else {
                if (pre_node->left_key != NULL && pre_node->right_key != NULL)
                    target_node = pre_node;
                break;
            }
        }

        update[i] = pre_node;
    }

    if (NULL == target_node){
        if (-1 == pre_node->index && cur_node != NULL)
            target_node = cur_node;
        else
            target_node = pre_node;
    }

    struct spx_block_skp_serial_context* serial_ctx = spx_block_skp_serial_context_new();
    //lock node 
    //pthread_mutex_lock(&target_node->mutex);
    int64_t index = spx_block_skp_serial(block_skp, key, value, target_node->index, serial_ctx); 
    if (-2 == index){
        //insert node successfully
        if (block_skp->cmp_key(target_node->left_key, key) > 0){
            block_skp->free_key(target_node->left_key);
            target_node->left_key = key;
            *p_key = NULL;
        } else if (block_skp->cmp_key(target_node->right_key, key) < 0){
            block_skp->free_key(target_node->right_key);
            target_node->right_key = key;
            *p_key = NULL;
        }
    } else if (index >= 0){
        if (serial_ctx->old_left_key != NULL && block_skp->cmp_key(serial_ctx->old_left_key, key) > 0){
            block_skp->free_key(serial_ctx->old_left_key);
            serial_ctx->old_left_key = key;
            *p_key = NULL;
        } else if (serial_ctx->new_right_key != NULL && block_skp->cmp_key(serial_ctx->new_right_key, key) < 0){
            block_skp->free_key(serial_ctx->new_right_key);
            serial_ctx->new_right_key = key;
            *p_key = NULL;
        } else if ((serial_ctx->old_right_key != NULL && block_skp->cmp_key(serial_ctx->old_right_key, key) < 0) && (serial_ctx->new_left_key != NULL && block_skp->cmp_key(serial_ctx->new_left_key, key) > 0)){
            block_skp->free_key(serial_ctx->new_left_key);
            serial_ctx->new_left_key = key;
            *p_key = NULL;
        }
        if (serial_ctx->old_left_key != NULL){
            block_skp->free_key(target_node->left_key);
            target_node->left_key = serial_ctx->old_left_key;
        }

        if (serial_ctx->old_right_key != NULL){
            block_skp->free_key(target_node->right_key);
            target_node->right_key = serial_ctx->old_right_key;
        }

        //split new block successfully
        //add new node
        void* new_left_key = serial_ctx->new_left_key;
        void* new_right_key = serial_ctx->new_right_key;

        //add head level
        int level = spx_skp_level_rand();

        if (level > block_skp->level){
            for (i = block_skp->level + 1; i <= level; i++) update[i] = block_skp->head;
            block_skp->level = level;
        }

        struct spx_block_skp_node *node = spx_block_skp_node_new(level, new_left_key, new_right_key, index);
        for (i = level; i >= 0; i--){
            node->next_node[i] = update[i]->next_node[i];
            update[i]->next_node[i] = node;
        }
        block_skp->length++;

    } else {
        if (serial_ctx->old_left_key != NULL)
            block_skp->free_key(serial_ctx->old_left_key);
        if (serial_ctx->old_right_key != NULL)
            block_skp->free_key(serial_ctx->old_right_key);
        if (serial_ctx->new_left_key != NULL)
            block_skp->free_key(serial_ctx->new_left_key);
        if (serial_ctx->new_right_key != NULL)
            block_skp->free_key(serial_ctx->new_right_key);
        printf("split new block failed\n");
        goto r1;
    }

    //pthread_mutex_unlock(&target_node->mutex);
    pthread_mutex_unlock(&block_skp->mutex);
    spx_block_skp_serial_context_free(&serial_ctx);
    return 0;

r1:
    spx_block_skp_serial_context_free(&serial_ctx);
    return -1;
}/*}}}*/

/*
 *spx_block_skp query
 */
//find the node in given block_skp by the key
int spx_block_skp_query(struct spx_block_skp *block_skp, void *key, struct spx_skp_query_result *result){ /*{{{*/
    if (NULL == block_skp){
        printf("block_skp is NULL in spx_block_skp_query\n");
        return -1;
    }

    if (NULL == key){
        printf("key is NULL in spx_block_skp_query\n");
        return -1;
    }

    if (NULL == result){
        printf("result is NULL in spx_block_skp_query\n");
        return -1;
    }

    //lock
    pthread_mutex_lock(&block_skp->mutex);
    struct spx_block_skp_node *cur_node = NULL, *pre_node =  block_skp->head;
    struct spx_block_skp_node *target_node = NULL;

    int i;
    //find the nodes to insert after
    for (i = block_skp->level; i >= 0; i--){
        while ((NULL == target_node) && ((cur_node = pre_node->next_node[i]) != NULL)){
            if ((cur_node->right_key != NULL) && (block_skp->cmp_key(key, cur_node->right_key) > 0))
                pre_node = cur_node; 
            else if ((cur_node->left_key != NULL) && (block_skp->cmp_key(key, cur_node->left_key) >= 0)){
                target_node = cur_node;
                break;
            }
            else 
                break;
        }

        if (target_node != NULL)
            break;
    }

    if (target_node != NULL){
        spx_block_skp_serial_node_query(block_skp, key, target_node->index, result);
        //this query need set in lock, because index may change by insert thread
    }

    //unkock
    pthread_mutex_unlock(&block_skp->mutex);

    return 0;
}/*}}}*/

/*
 *block skiplist and split
 */
pthread_t spx_block_skp_thread_new(SpxLogDelegate *log, err_t *err){/*{{{*/
    pthread_t tid = 0;
    if (0 != (*err = pthread_create(&tid, NULL, (void *)spx_block_skp_handler, log))){
        SpxLog2(log, SpxLogError, *err, "thread spx_block_skp_handler failed");
        return 0;
    }

    return tid;
}/*}}}*/

static void spx_block_skp_handler(void *arg){/*{{{*/
    while (1) {
        struct spx_block_skp_index* idx_lst = spx_block_skp_config_load_index();
        struct spx_block_skp_index* tmp_idx = idx_lst->next_idx;
        while (tmp_idx != NULL) {
            struct logdb_queue* task_queue = spx_block_skp_config_task_queue_dispatcher(tmp_idx->idx);
            //printf("revolver queue:%s ===> size:%zd\n", task_queue->name, task_queue->size);
            struct spx_block_skp_task* task = logdb_queue_pop(task_queue);
            if (task != NULL)
                pool_task_add(spx_block_skp_insert_task, task);
            tmp_idx = tmp_idx->next_idx;
        }
    }
}/*}}}*/

static void spx_block_skp_insert_task(void *arg){/*{{{*/
    if (NULL == arg){
        printf("arg is NULL in spx_block_skp_insert_task\n");
        return;
    }
    struct spx_block_skp_task* task = (struct spx_block_skp_task*)arg;
    
    if (task != NULL){
        printf("exec task:%s\n", task->skp->name);
        spx_block_skp_insert(task->skp, &task->key, task->value);
    }

    spx_block_skp_config_task_pool_push(task);
}/*}}}*/
