/*************************************************************************
> File Name: struct spx_block_skp.c
> Author: shuaixiang
> Mail: shuaixiang@yuewen.com
> Created Time: Mon 07 Sep 2015 03:13:11 AM UTC
************************************************************************/
#include <ev.h>
#include <pthread.h>
#include "spx_block_skp.h"
#include "spx_skp_serial.h"
#include "spx_string.h"
#include "spx_log.h"

#define SpxSkpIndexConf "./skiplist/config/index.config" 
#define SpxSkpMaxLen 10

#define NewBlockNode(n) ((struct spx_block_skp_node *) calloc(1, sizeof(struct spx_block_skp_node) + n*sizeof(struct spx_block_skp_node*)))

//private method declare
struct block_context{
    ev_async async_watcher;
    struct spx_skp *skp;

    struct block_context *pre_ctx;
    struct block_context *next_ctx;
};

static void spx_block_skp_register_async_watcher(ev_async *watcher, void (*cb)(struct ev_loop *loop, ev_async *watcher, int revents), void *data);
static int spx_block_skp_node_free(struct spx_block_skp *block_skp, struct spx_block_skp_node *block_node);
static err_t spx_block_skp_ctx_pool_free();
static size_t spx_block_skp_get_ctx_count();
static err_t spx_block_skp_ctx_push(struct block_context *ctx);
static struct block_context * spx_block_skp_ctx_pop();
static  void spx_block_skp_handler(void *arg);
static void spx_block_skp_event_insert(EV_P_ ev_async *watcher, int revents);
static void spx_block_skp_wait_cb(struct ev_loop *loop, ev_io *w, int revents);
static struct spx_block_skp * spx_block_skp_new(spx_skp_type key_type, SpxSkpCmpDelegate cmp_key, spx_skp_type value_type, SpxSkpCmpDelegate cmp_value, const char * block_skp_name, SpxSkpFreeDelegate free_key);
static int spx_block_skp_destory(struct spx_block_skp ** p_block_skp);

struct block_context * g_block_ctx_top = NULL;
static struct ev_loop * g_skp_block_loop = NULL;

/*
 * private method
 */

static void spx_block_skp_register_async_watcher(ev_async *watcher, void (*cb)(struct ev_loop *loop, ev_async *watcher, int revents), void *data){/*{{{*/
    SpxZero(*watcher);
    ev_async_init(watcher, cb);
    watcher->data = data;
}/*}}}*/

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
 * block_context pool  operation
 */
err_t spx_block_skp_ctx_pool_new(size_t ctx_pool_size){/*{{{*/
    err_t err = 0;
    uint32_t i = 0;
    
    for(i = 0; i < ctx_pool_size; i++){
        struct block_context *ctx = (struct block_context *) malloc(sizeof(*ctx));
        if (NULL == ctx){
            printf("malloc block_context failed\n");
            err = -1;
            break;
        }

        ctx->skp = NULL;
        ctx->pre_ctx = NULL;
        ctx->next_ctx = NULL;

        if (NULL == g_block_ctx_top){
            g_block_ctx_top = ctx;
        } else {
            g_block_ctx_top->next_ctx = ctx;
            ctx->pre_ctx = g_block_ctx_top; 
            g_block_ctx_top = ctx;
        }
    }

    return err;
}/*}}}*/

static err_t spx_block_skp_ctx_pool_free(){/*{{{*/
    err_t err = 0;

    while (g_block_ctx_top != NULL){
        struct block_context * ctx = g_block_ctx_top;
        g_block_ctx_top = ctx->pre_ctx;
        free(ctx->skp);
        free(ctx);
    }
    
    return err;
}/*}}}*/

static size_t spx_block_skp_get_ctx_count(){/*{{{*/
    size_t count = 0;
    struct block_context *ctx = g_block_ctx_top;
    while (ctx != NULL){
        count++;
        ctx = ctx->pre_ctx;
    }

    return count;
}/*}}}*/

static err_t spx_block_skp_ctx_push(struct block_context *ctx){/*{{{*/
    if (NULL == ctx){
        printf("ctx is NULL\n");
        return -1;
    }

    spx_skp_destory(ctx->skp);
    if (NULL == g_block_ctx_top){
        g_block_ctx_top = ctx; 
        ctx->next_ctx = NULL;
        ctx->pre_ctx = NULL;
    } else {
        g_block_ctx_top->next_ctx = ctx;
        ctx->pre_ctx = g_block_ctx_top;
        g_block_ctx_top = ctx;
    }

    return 0;
}/*}}}*/

static struct block_context * spx_block_skp_ctx_pop(){/*{{{*/
    struct block_context * ctx = g_block_ctx_top;
    if (ctx != NULL){
        g_block_ctx_top = ctx->pre_ctx; 
        if (g_block_ctx_top != NULL){
            g_block_ctx_top->next_ctx = NULL;
        }
    }

    return ctx;
}/*}}}*/

/*
 * skp_list operation
 */
//read config and init skp_name
struct spx_skp_idx * spx_skp_read_config()/*{{{*/
{
    if (g_spx_skp_idx_head.next_idx == NULL){
        FILE *fp = fopen(SpxSkpIndexConf, "r");
        if (NULL == fp){
            printf("open %s failed\n", SpxSkpIndexConf);
            return NULL;
        }

        char line[100];
        struct spx_skp_idx *tmp_idx = &g_spx_skp_idx_head;
        while(fgets(line, sizeof(line), fp)){
            if('#' != *line && '\n' != *line){
                int i = 0;
                for(i = 0; i < (int)strlen(line); i++){
                    if( *(line + i) == ':')
                    break;
                }

                struct spx_skp_idx * new_idx = (struct spx_skp_idx *) malloc(sizeof(struct spx_skp_idx));           
                new_idx->next_idx = NULL;
                new_idx->index = (char *) malloc(sizeof(char) * (i + 1));
                strncpy(new_idx->index, line, i);
                *(new_idx->index + i) = '\0';
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

    return &g_spx_skp_idx_head;
}/*}}}*/

 //spx_skp_list add
struct spx_skp_list_node *spx_skp_list_add(struct spx_skp *skp){/*{{{*/
    struct spx_skp_list *skp_list = &g_spx_skp_list;
    if (NULL == skp_list){
        printf("list is NULL\n");
        return NULL;
    }

    if (NULL == skp){
        printf("skp is NULL\n");
        return NULL;
    }

    struct spx_skp_list_node *new_skp = (struct spx_skp_list_node *) calloc(1, sizeof(struct spx_skp_list_node));
    printf("************new skp_node*****************\n");
    new_skp->skp = skp;
    struct spx_skp_list_node *tmp_skp_node = skp_list->tail;

    if (NULL == tmp_skp_node){
        skp_list->head = new_skp;
        skp_list->tail = new_skp;
    } else {
        tmp_skp_node->next_skp = new_skp;
        skp_list->tail = new_skp;
    }

    return new_skp;
}/*}}}*/

 //spx_skp_list delete, ATTENTION: spx_skp in spx_skp_list_node is not free, it is move to the freeze list
int spx_skp_list_delete(struct spx_skp_list *skp_list, char *skp_name){/*{{{*/
    if (NULL == skp_list){
        printf("list is NULL\n");
        return -1;
    }

    if (NULL == skp_name){
        printf("skp is NULL in spx_skp_list_delete\n");
        return -1;
    }

    struct spx_skp_list_node *tmp_skp_node = skp_list->head;
    struct spx_skp_list_node *pre_skp_node = skp_list->head;
    if (NULL == tmp_skp_node){
        printf("tmp_skp_node is NULL\n");
        return -1;
    }

    while (tmp_skp_node){
        if ( (tmp_skp_node->skp != NULL) && (tmp_skp_node->skp->name != NULL) && !strncmp(tmp_skp_node->skp->name, skp_name, strlen(skp_name))){
            if (tmp_skp_node == pre_skp_node){
                skp_list->head = tmp_skp_node->next_skp;
                if (NULL == tmp_skp_node->next_skp)
                    skp_list->tail = NULL;
            } else {
                pre_skp_node->next_skp = tmp_skp_node->next_skp;
                if (NULL == tmp_skp_node->next_skp)
                    skp_list->tail = pre_skp_node;
            }

            printf("************free skp_node****************\n");
            free(tmp_skp_node);
            return 0;
        }

        pre_skp_node = tmp_skp_node;
        tmp_skp_node = tmp_skp_node->next_skp; 
    }

    return -1;
}/*}}}*/

//spx_skp_list query
struct spx_skp * spx_skp_list_get(char *skp_name, err_t *err){/*{{{*/
    struct spx_skp_list *skp_list = &g_spx_skp_list;
    struct spx_skp_list_node *tmp_skp_node = skp_list->head;
    if (NULL == tmp_skp_node){
        printf("tmp_skp is NULL\n");
        return NULL;
    }

    while (tmp_skp_node){
        if ((tmp_skp_node->skp->name != NULL) && !strncmp(tmp_skp_node->skp->name, skp_name, strlen(skp_name))){
            if (tmp_skp_node->skp->length < SpxSkpMaxLen){
                return tmp_skp_node->skp; 
            } else {
                struct block_context *ctx = spx_block_skp_ctx_pop();
                if (NULL == ctx){
                    *err = -1;
                    return tmp_skp_node->skp;
                }

                ctx->skp = tmp_skp_node->skp;
                spx_block_skp_register_async_watcher(&ctx->async_watcher, spx_block_skp_event_insert, ctx);
                ev_async_start(g_skp_block_loop, &ctx->async_watcher);
                printf("send skp:%s size:%zd\n", ctx->skp->name, ctx->skp->length);
                spx_skp_list_delete(&g_spx_skp_list, skp_name);
                ev_async_send(g_skp_block_loop, &ctx->async_watcher);
                return NULL;
            }
        }

        tmp_skp_node = tmp_skp_node->next_skp; 
    }

    return NULL;
}/*}}}*/

/*
 * block_skp_list operation
 */

 //spx_block_skp_list add
struct spx_block_skp_list_node *spx_block_skp_list_add(struct spx_block_skp *block_skp){/*{{{*/
    struct spx_block_skp_list *block_skp_list = &g_spx_block_skp_list;
    if (NULL == block_skp_list){
        printf("block_skp_list is NULL\n");
        return NULL;
    }

    if (NULL == block_skp){
        printf("block_skp is NULL\n");
        return NULL;
    }

    struct spx_block_skp_list_node *new_block_skp_node = (struct spx_block_skp_list_node *) calloc(1, sizeof(struct spx_block_skp_list_node));
    new_block_skp_node->block_skp = block_skp;
    struct spx_block_skp_list_node *tmp_block_skp_node = block_skp_list->tail;

    if (NULL == tmp_block_skp_node){
        block_skp_list->head = new_block_skp_node;
        block_skp_list->tail = new_block_skp_node;
    } else {
        tmp_block_skp_node->next_block_skp = new_block_skp_node;
        block_skp_list->tail = new_block_skp_node;
    }

    return new_block_skp_node;
}/*}}}*/

//spx_block_skp_list query
struct spx_block_skp * spx_block_skp_list_get(char *block_skp_name, spx_skp_type key_type, SpxSkpCmpDelegate cmp_key, spx_skp_type value_type, SpxSkpCmpDelegate cmp_value, SpxSkpFreeDelegate free_key){/*{{{*/
    struct spx_block_skp_list *block_skp_list = &g_spx_block_skp_list;
    struct spx_block_skp_list_node *tmp_block_skp_node = block_skp_list->head;
    if (tmp_block_skp_node != NULL){
        while (tmp_block_skp_node){
            if ( (tmp_block_skp_node->block_skp != NULL) && (tmp_block_skp_node->block_skp->name != NULL) && !strncmp(tmp_block_skp_node->block_skp->name, block_skp_name, strlen(block_skp_name))){
                    return tmp_block_skp_node->block_skp; 
            }
            tmp_block_skp_node = tmp_block_skp_node->next_block_skp; 
        }
    }

    struct spx_block_skp *block_skp = spx_block_skp_new(key_type, cmp_key, value_type, cmp_value, block_skp_name, free_key);
    if (spx_block_skp_unserial(block_skp) != 0){
        printf("unserial block_skp:%s failed or not built yet\n", block_skp_name);
        spx_block_skp_destory(&block_skp);
    } else {
        spx_block_skp_list_add(block_skp);
        return block_skp;
    }

    return NULL;
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
            }
            else {
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
static struct spx_block_skp * spx_block_skp_new(spx_skp_type key_type, SpxSkpCmpDelegate cmp_key, spx_skp_type value_type, SpxSkpCmpDelegate cmp_value, const char * block_skp_name, SpxSkpFreeDelegate free_key){/*{{{*/
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

    if (NULL == free_key){
        block_skp->free_key = spx_skp_default_free;
    } else {
        block_skp->free_key = free_key;
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
static int spx_block_skp_destory(struct spx_block_skp ** p_block_skp){/*{{{*/
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
int spx_block_skp_insert(struct spx_block_skp * block_skp, void * key, void *value){/*{{{*/
    if (block_skp == NULL){
        printf("block_skp is NULL\n");
        return -1;
    }

    struct spx_block_skp_node *update[SpxSkpMaxLevel];
    struct spx_block_skp_node *cur_node = NULL, *pre_node = block_skp->head;
    struct spx_block_skp_node *target_node = NULL;
    int64_t off = 0;
    int64_t len = 0;

    int i;
    //find the nodes to insert after
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

    //insert node 
    int64_t index = spx_block_skp_node_serial(block_skp, key, value, target_node->index); 
    if (-2 == index){
        //insert node successfully
        if (g_spx_block_skp_node_serial_ctx.old_left_key != NULL){
            block_skp->free_key(target_node->left_key);
            target_node->left_key = g_spx_block_skp_node_serial_ctx.old_left_key;
        }
        
        if (g_spx_block_skp_node_serial_ctx.old_right_key != NULL){
            block_skp->free_key(target_node->right_key);
            target_node->right_key = g_spx_block_skp_node_serial_ctx.old_right_key;
        }

        return 0;
    } else if (index >= 0){
        if (g_spx_block_skp_node_serial_ctx.old_left_key != NULL){
            block_skp->free_key(target_node->left_key);
            target_node->left_key = g_spx_block_skp_node_serial_ctx.old_left_key;
        }

        if (g_spx_block_skp_node_serial_ctx.old_right_key != NULL){
            block_skp->free_key(target_node->right_key);
            target_node->right_key = g_spx_block_skp_node_serial_ctx.old_right_key;
        }

        if (NULL == g_spx_block_skp_node_serial_ctx.new_left_key || NULL == g_spx_block_skp_node_serial_ctx.new_right_key){
            if (g_spx_block_skp_node_serial_ctx.old_left_key != NULL){
                block_skp->free_key(g_spx_block_skp_node_serial_ctx.old_left_key);
            }

            if (g_spx_block_skp_node_serial_ctx.old_right_key != NULL){
                block_skp->free_key(g_spx_block_skp_node_serial_ctx.old_right_key);
            }

            printf("split a new block , but  new_left_key or new_right_key is NULL\n");
            return -1;
        }

        //split new block successfully
        //add new node
        void *new_left_key = g_spx_block_skp_node_serial_ctx.new_left_key;
        void *new_right_key = g_spx_block_skp_node_serial_ctx.new_right_key;

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
        return 0;
    } else {
        printf("split new block failed\n");
        return -1;
    }
}/*}}}*/

err_t spx_block_skp_merge(struct spx_block_skp *block_skp, struct spx_skp *skp){/*{{{*/
    if (NULL == block_skp){
        printf("block_skp is NULL\n");
        return -1;
    }

    if (NULL == skp){
        printf("skp is NULL\n");
        return -1;
    }

    struct spx_skp_node *cur_skp_node = skp->head->next_node[0];
    while (cur_skp_node){
        struct spx_skp_node_value *tmp_nv = cur_skp_node->value;
        while (tmp_nv != NULL){
            spx_block_skp_insert(block_skp, cur_skp_node->key, tmp_nv->value); 
            tmp_nv = tmp_nv->next_value;
        }

        cur_skp_node = cur_skp_node->next_node[0];
    }

    return 0;
}/*}}}*/

/*
 *spx_block_skp query
 */
//find the node in given block_skp by the key
struct spx_skp_query_result *spx_block_skp_query(struct spx_block_skp *block_skp, void *key){ /*{{{*/
    if (NULL == key){
        printf("key is NULL\n");
        return NULL;
    }
    struct spx_block_skp_node *cur_node = NULL, *pre_node =  block_skp->head;
    struct spx_skp_query_result *result = NULL;
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
        result = spx_block_skp_serial_node_query(block_skp, key, target_node->index);
    }

    return result;
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

static void spx_block_skp_wait_cb(struct ev_loop *loop, ev_io *w, int revents){/*{{{*/
    ev_io_stop(loop, w);
    ev_io_start(loop, w);
}/*}}}*/

static void spx_block_skp_handler(void *arg){/*{{{*/
    SpxLogDelegate *log = (SpxLogDelegate *) arg;
    //g_skp_block_loop = ev_loop_new(0);    
    g_skp_block_loop = EV_DEFAULT;
    if (NULL == g_skp_block_loop){
        SpxLog1(log, SpxLogError, "ev_loop_new failed");
        return;
    }

    //set a wait_io in loop, that will make loop not stop 
    ev_io wait_io;
    ev_io_init(&wait_io, spx_block_skp_wait_cb, STDIN_FILENO, EV_READ);
    ev_io_start(g_skp_block_loop, &wait_io);
    ev_run(g_skp_block_loop, 0);
}/*}}}*/

static void spx_block_skp_event_insert(EV_P_ ev_async *watcher, int revents){/*{{{*/
    ev_async_stop(g_skp_block_loop, watcher);
    struct block_context *ctx = (struct block_context *) watcher->data;
    if (NULL == ctx){
        printf("ctx is NULL\n");
        return;
    }

    struct spx_skp * skp = ctx->skp;
    printf("merge skp:%s size:%zd\n", skp->name, skp->length);
    struct spx_block_skp *block_skp = spx_block_skp_list_get(skp->name, skp->key_type, skp->cmp_key, skp->value_type, skp->cmp_value, skp->free_key);
    if (block_skp != NULL ){
        spx_block_skp_merge(block_skp, skp);
        printf("############################ SKP_LEN: %d ###############################", (int)block_skp->length);
    }

    spx_block_skp_ctx_push(ctx);
}/*}}}*/
