/*************************************************************************
    > File Name: msgpk_tree.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Fri 22 Apr 2016 03:19:49 AM UTC
 ************************************************************************/

#include "msgpk_tree.h"
#include "msgpk_build_tree.h"

#define MsgpkTreeKeySize 100
#define MsgpkTreeUnidSize 100

//declare
static void msgpk_tree_feed_query_result(struct spx_skp *skp, int *uni_count, struct spx_skp_query_result *res);
static void msgpk_tree_feed_metadata_list(struct spx_skp *skp, int *uni_count, struct spx_skp_serial_metadata_list *md_lst);
static struct spx_skp_serial_metadata_list *msgpk_tree_merge_intersection(struct spx_skp *skp, SpxSkpFreeDelegate skp_key_free);
static struct spx_skp_serial_metadata_list *msgpk_tree_merge_union(struct spx_skp *skp);
static char msgpk_tree_parse_key(char * obj_key, char *key);
static void msgpk_tree_query_exec(struct msgpk_object *obj, struct spx_skp *parent_skp, int *uni_count);
static void msgpk_tree_metadata_free(void *mdp);
static int msgpk_tree_level_equal_query(void *query_key, struct spx_skp *skp, struct spx_block_skp *block_skp, struct spx_skp_query_result *result);

static void msgpk_tree_metadata_free(void *mdp);
static void *msgpk_tree_metadata_copy(void *src);

static void msgpk_tree_metadata_free(void *mdp){/*{{{*/
    if (NULL == mdp){
        printf("mdp is NULL in msgpk_tree_metadata_copy\n");
        return;
    }

    struct spx_skp_serial_metadata * md = (struct spx_skp_serial_metadata *) mdp;
    spx_skp_serial_md_free(md);
}/*}}}*/

static void *msgpk_tree_metadata_copy(void *src){/*{{{*/
    struct spx_skp_serial_metadata * new_md = spx_skp_serial_md_copy(src);
    if (NULL == new_md){
        printf("copy md failed in msgpk_tree_metadata_copy\n");
    }
    return new_md;
}/*}}}*/

/*
 * query method
 */
static void msgpk_tree_feed_query_result(struct spx_skp *skp, int *uni_count, struct spx_skp_query_result *res){/*{{{*/
    if (NULL == res){
        printf("result is NULLin msgpk_tree_feed_query_result\n");
        return;
    }

    struct spx_skp_query_result_node *tmp_result_node = res->head;
    
    while (tmp_result_node){
        void *value = tmp_result_node->value;
        if (NULL != value) {
            int *count = (int *) malloc(sizeof(int));
            *count = *uni_count;
            spx_skp_insert(skp, value, count);
            (*uni_count)++;//unique insert to make sure size of skp node will increase of the same key 
        } else {
            printf("value is NULL");
        }
        tmp_result_node = tmp_result_node->next_result_node;
    }

}/*}}}*/

static void msgpk_tree_feed_metadata_list(struct spx_skp *skp, int *uni_count, struct spx_skp_serial_metadata_list *md_lst){/*{{{*/
    if (NULL == md_lst){
        printf("mdl is NULL\n");
        return;
    }
    if (NULL == md_lst->head){
        printf("md_lst->head is NULL\n");
        return;
    }

    struct spx_skp_serial_metadata_list_node *tmp_md = md_lst->head;
    while (tmp_md){
        if(NULL != tmp_md){
            int *count = (int *) malloc(sizeof(int));
            *count = *uni_count;
            spx_skp_insert(skp, tmp_md->md, count);
            (*uni_count)++;//keep value is unique, then value size can increase
        } else {
            printf("tmp_md is NULL");
        }
        tmp_md = tmp_md->next_md;
    }

    spx_skp_serial_metadata_list_free(md_lst, false);
}/*}}}*/

static struct spx_skp_serial_metadata_list *msgpk_tree_merge_intersection(struct spx_skp *skp, SpxSkpFreeDelegate skp_key_free){/*{{{*/
    if(NULL == skp->head){
        printf("skp->head is NULL\n");        
        return NULL;
    }

    struct spx_skp_node * cur_node = skp->head->next_node[0];
    struct spx_skp_serial_metadata_list * md_lst = spx_skp_serial_metadata_list_new();

    while (cur_node) {
        if (cur_node->size > 1) {
            spx_skp_serial_metadata_list_insert(md_lst, cur_node->key);
        } else {
            skp_key_free(cur_node->key);//the exclude key can be free here
        }
        cur_node = cur_node->next_node[0];
    }

    spx_skp_destory(skp);

    return md_lst;
}/*}}}*/

static struct spx_skp_serial_metadata_list *msgpk_tree_merge_union(struct spx_skp *skp){/*{{{*/
    if(NULL == skp->head){
        printf("skp->head is NULL\n");        
        return NULL;
    }

    struct spx_skp_node * cur_node = skp->head->next_node[0];
    struct spx_skp_serial_metadata_list * md_lst = spx_skp_serial_metadata_list_new();

    while (cur_node) {
            spx_skp_serial_metadata_list_insert(md_lst, cur_node->key);
            cur_node = cur_node->next_node[0];
    }

    spx_skp_destory(skp);
    return md_lst;
}/*}}}*/

static char msgpk_tree_parse_key(char * obj_key, char *key){/*{{{*/
   int len = strlen(obj_key); 
   char op = *(obj_key + len -1);
   strncpy(key, obj_key, len-1);
   *(key + len - 1) = '\0';
   if('=' == op){
        return 0;
   } else if('<' == op){
        return 1;
   } else if('>' == op){
        return 2;
   } else{
       return -1;
   }
}/*}}}*/

static int msgpk_tree_level_equal_query(void *query_key, struct spx_skp *skp, struct spx_block_skp *block_skp, struct spx_skp_query_result *result){/*{{{*/
    if (NULL == query_key){
        printf("query_key is NULL in msgpk_tree_level_equal_query\n");
        return -1;
    }

    if (NULL == skp){
        printf("skp is NULL in msgpk_tree_level_equal_query\n");
    } else {
        //query in insert_skp, no need to lock for single thread
        spx_skp_query(skp, query_key, msgpk_tree_metadata_copy, result);
    }


    //query in freezen_skp
    spx_skp_queue_lock();

    struct spx_skp_list *freezen_skp_list = spx_skp_queue_visit();
    struct spx_skp_list_node *freezen_skp_node = freezen_skp_list->head;

    while (freezen_skp_node != NULL){
        struct spx_skp *freezen_skp = freezen_skp_node->skp;
        spx_skp_query(freezen_skp, query_key, msgpk_tree_metadata_copy, result);
        freezen_skp_node = freezen_skp_node->next_skp;
    }

    spx_skp_list_destory(&freezen_skp_list);
    
    //query in block_skp
    if (NULL == block_skp){
        printf("block_skp is NULL in msgpk_tree_level_equal_query\n");
    } else {
        spx_block_skp_query(block_skp, query_key, result);
    }

    spx_skp_queue_unlock();

    return 0;
}/*}}}*/

static int msgpk_tree_level_smaller_query(void *query_key, struct spx_skp *skp, struct spx_block_skp *block_skp, struct spx_skp_query_result *result){
    return 0;
}

static int msgpk_tree_level_biger_query(void *query_key, struct spx_skp *skp, struct spx_block_skp *block_skp, struct spx_skp_query_result *result){
    return 0;
}

static void msgpk_tree_query_exec(struct msgpk_object *obj, struct spx_skp *parent_skp, int *uni_count){//finally, the root_skp contains the query result/*{{{*/
    if (NULL == obj){
        printf("obj is NULL in msgpk_tree_query\n");
        return;
    }

    err_t err = 0;
    object_value ov_key = obj->key;
    object_value ov_value = obj->value;

    if(obj->obj_type == OBJ_TYPE_MAP){
        struct spx_skp *tmp_skp = spx_skp_new_tmp(cmp_md, cmp_int, "tmp_skp", msgpk_tree_metadata_free, NULL, false);
        struct spx_skp_serial_metadata_list *tmp_md_lst = NULL;

        //if obj->child != NULL, obj_type = OBJ_TYPE_MAP
        if (NULL != obj->child){
                int tmp_uni_count = 0;
                msgpk_tree_query_exec(obj->child, tmp_skp, &tmp_uni_count);
        }

        if (*ov_key.str_val == '&'){
           tmp_md_lst = msgpk_tree_merge_intersection(tmp_skp, msgpk_tree_metadata_free); 
        }
        else if(*ov_key.str_val == '|'){
           tmp_md_lst = msgpk_tree_merge_union(tmp_skp); 
        } else {
            printf("no such query mark %s\n", ov_value.str_val);
            return;
        }
        
        msgpk_tree_feed_metadata_list(parent_skp, uni_count, tmp_md_lst);
    } else {
        SpxSkpCmpDelegate cmp_key;
        SpxSkpCmpDelegate cmp_value = cmp_md;
        spx_skp_type key_type = SKP_TYPE_STR;
        spx_skp_type value_type = SKP_TYPE_MD;

        void *query_key;
        switch(obj->obj_type){
            case OBJ_TYPE_STR://string
                cmp_key = cmp_str;
                key_type = SKP_TYPE_STR;
                query_key = ov_value.str_val;
                break;
            case OBJ_TYPE_POSITIVE_INT:
            case OBJ_TYPE_NEGATIVE_INT:
            case OBJ_TYPE_INT32://int
                cmp_key = cmp_int;
                key_type = SKP_TYPE_INT;
                query_key = &ov_value.int32_val;
                break;
            case OBJ_TYPE_INT64://long
                cmp_key = cmp_long;
                key_type = SKP_TYPE_LONG;
                query_key = &ov_value.int64_val;
                break;
            case OBJ_TYPE_FLOAT:
                cmp_key = cmp_float;
                key_type = SKP_TYPE_FLOAT;
                query_key = &ov_value.float_val;
            case OBJ_TYPE_DOUBLE:
                cmp_key = cmp_double;
                key_type = SKP_TYPE_DOUBLE;
                query_key = &ov_value.double_val;
            default:
                query_key = NULL;
                printf("object_value type is not supported for msgpk_tree_query yet\n");
                return;
        }

        char index_name[MsgpkTreeKeySize];
        int op = msgpk_tree_parse_key(ov_key.str_val, index_name);        
        if(-1 == op){
            printf("msgpk_tree_parse_key error");
            return;
        }

        struct spx_skp *skp = spx_skp_list_get(&g_spx_skp_list, index_name, &err);
        struct spx_block_skp *block_skp = spx_block_skp_list_get(index_name, key_type, cmp_key, value_type, cmp_value, NULL);//unserial in it

        struct spx_skp_query_result *res = spx_skp_query_result_new();
        switch (op) {
            case 0:
                msgpk_tree_level_equal_query(query_key, skp, block_skp, res);
                break;
            case 1:
                spx_skp_smaller_query(skp, query_key, msgpk_tree_metadata_copy, res);
                break;
            case 2:
                spx_skp_bigger_query(skp, query_key, msgpk_tree_metadata_copy, res);
                break;
            default:
                printf("op is error\n");
                return;
        }

        if(NULL != res){
            msgpk_tree_feed_query_result(parent_skp, uni_count, res); 
            spx_skp_query_result_destory(res);
        }
    }

    if(NULL != obj->next){
        msgpk_tree_query_exec(obj->next, parent_skp, uni_count);
    }
}/*}}}*/

struct spx_skp_serial_metadata_list *msgpk_tree_query(struct msgpk_object * obj){/*{{{*/
    if(NULL == obj->child){
        printf("obj->child is NULL\n");
        return NULL;
    }

    int uni_count = 0;
    struct spx_skp *root_skp = spx_skp_new_tmp(cmp_md, cmp_int, "root_skp", msgpk_tree_metadata_free, NULL, false); 
    msgpk_tree_query_exec(obj->child, root_skp, &uni_count);
    return msgpk_tree_merge_union(root_skp); 
}/*}}}*/

char *msgpk_tree_add(struct msgpk_object *root, size_t req_size, char *request){/*{{{*/
    struct spx_block_skp_index *tmp_idx = spx_block_skp_load_index(); 
    err_t err = 0;
    if (NULL == tmp_idx){
        printf("read index config error\n");
        return NULL;
    }

    struct spx_skp_serial_metadata *md = spx_skp_serial_data_writer2md((const ubyte_t*)request, req_size);
    if(NULL == md){
        perror("data_writer2md:");
        return NULL;
    }

    char *unid = (char *) malloc(sizeof(char) * MsgpkTreeUnidSize);
    spx_skp_serial_gen_unid(md, unid, MsgpkTreeUnidSize);

    while (tmp_idx->next_idx){
        tmp_idx = tmp_idx->next_idx;
        char *index_name = tmp_idx->index;
        SpxSkpCmpDelegate cmp_key; 
        SpxSkpCmpDelegate cmp_value = cmp_md;
        object_value kv;
        kv.str_val = index_name;
        object_type kt = OBJ_TYPE_STR;
        void *key = NULL;
        spx_skp_type key_type = SKP_TYPE_STR;
        spx_skp_type value_type = SKP_TYPE_MD;

        struct msgpk_object *obj = msgpk_tree_find_rule_node(root, 1, kt, kv);
        if (NULL == obj){
            printf("not found index_name: %s\n", index_name);
            continue;
        }

        switch(tmp_idx->type){
            case 0:
                cmp_key = cmp_str;
                key = (char *) calloc(1, sizeof(char) * (obj->obj_len + 1));
                strncpy(key, obj->value.str_val, obj->obj_len); 
                key_type = SKP_TYPE_STR;
                break;
            case 1:
                cmp_key = cmp_int;
                key = (int *) malloc(sizeof(int));
                *(int *)key = obj->value.int32_val;
                key_type = SKP_TYPE_INT;
                break;
            case 2:
                cmp_key = cmp_long;
                key = (int64_t *) malloc(sizeof(int64_t));
                *(int64_t *)key = obj->value.int64_val;
                key_type = SKP_TYPE_LONG;
                break;
            default:
                printf("not support type of index_name\n");
                continue;
        }

        struct spx_skp *skp = spx_skp_list_get_push_queue(&g_spx_skp_list, index_name, &err);

        if(NULL == skp){
            skp = spx_skp_new(key_type, cmp_key, value_type, cmp_value, index_name, NULL, msgpk_tree_metadata_free);
            if (NULL == skp){
                printf("spx_skp_new failed, skp is NULL\n");
                return NULL;
            }

            spx_skp_list_add(&g_spx_skp_list, skp);
        }


        struct spx_skp_serial_metadata *new_md = spx_skp_serial_md_copy(md);
        spx_skp_insert(skp, key, new_md); 
    }

    spx_skp_serial_md_free(md);
    return unid;
}/*}}}*/
