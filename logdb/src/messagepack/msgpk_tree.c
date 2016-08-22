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
static void msgpk_tree_fill_node_value(struct spx_skp *skp, struct spx_skp_value_list *vl);
static void msgpk_tree_fill_mdlist(struct spx_skp *skp, struct spx_skp_serial_md_list *mdl);
static struct spx_skp_serial_md_list *msgpk_tree_merge_intersection(struct spx_skp *skp);
static struct spx_skp_serial_md_list *msgpk_tree_merge_union(struct spx_skp *skp);
static char msgpk_tree_parse_key(char * obj_key, char *key);
static void msgpk_tree_query_exec(struct msgpk_object *obj, struct spx_skp *parent_skp);
static void msgpk_tree_free_md(void *mdp);

static void msgpk_tree_free_md(void *mdp){/*{{{*/
    struct spx_skp_serial_metadata * md = (struct spx_skp_serial_metadata *) mdp;
    spx_skp_serial_free_md(md);
}/*}}}*/

/*
 * query method
 */
static void msgpk_tree_fill_node_value(struct spx_skp *skp, struct spx_skp_value_list *vl){/*{{{*/
    if (NULL == vl){
        printf("vl is NULL\n");
        return;
    }

    int count = 0;
    struct spx_skp_value_list_node *tmp_value_list_node = vl->head;
    
    while(tmp_value_list_node){
        struct spx_skp_node_value *v = tmp_value_list_node->value;
        while(v){
            if(NULL != v->value){
                spx_skp_insert(skp, v->value, &count);
                count++;
            } else{
                printf("v->value is NULL");
            }
            v = v->next_value;
        }
        tmp_value_list_node = tmp_value_list_node->next_value_list_node;
    }
}/*}}}*/

static void msgpk_tree_fill_mdlist(struct spx_skp *skp, struct spx_skp_serial_md_list *md_list){/*{{{*/
    if (NULL == md_list){
        printf("mdl is NULL\n");
        return;
    }

    if (NULL == md_list->head){
        printf("md_list->head is NULL\n");
        return;
    }

    int count = 0;
    struct spx_skp_serial_md_list_node *tmp_md = md_list->head;
    
    while (tmp_md){
        if(NULL != tmp_md){
            spx_skp_insert(skp, tmp_md->md, &count);
            count++;//keep value is unique, then value size can increase
        } else {
            printf("tmp_md is NULL");
        }

        tmp_md = tmp_md->next_md;
    }

    spx_skp_serial_md_list_free(md_list);
}/*}}}*/

static struct spx_skp_serial_md_list *msgpk_tree_merge_intersection(struct spx_skp *skp){/*{{{*/
    if(NULL == skp->head){
        printf("skp->head is NULL\n");        
        return NULL;
    }

    struct spx_skp_node * cur_node = skp->head->next_node[0];
    struct spx_skp_serial_md_list * md_list = spx_skp_serial_md_list_new();

    while (cur_node){
        if(cur_node->size > 1){
            spx_skp_serial_md_list_insert(md_list, cur_node->key);
        }
        cur_node = cur_node->next_node[0];
    }

    spx_skp_destory(skp);

    return md_list;
}/*}}}*/

static struct spx_skp_serial_md_list *msgpk_tree_merge_union(struct spx_skp *skp){/*{{{*/
    if(NULL == skp->head){
        printf("skp->head is NULL\n");        
        return NULL;
    }

    struct spx_skp_node * cur_node = skp->head->next_node[0];
    struct spx_skp_serial_md_list * md_list = spx_skp_serial_md_list_new();

    while (cur_node){
            spx_skp_serial_md_list_insert(md_list, cur_node->key);
            cur_node = cur_node->next_node[0];
    }

    spx_skp_destory(skp);

    return md_list;
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

static void msgpk_tree_query_exec(struct msgpk_object *obj, struct spx_skp *parent_skp){//finally, the root_skp contains the query result/*{{{*/
    if (NULL == obj){
        printf("obj is NULL in msgpk_tree_query\n");
        return;
    }

    object_value ov_key = obj->key;
    object_value ov_value = obj->value;

    if(obj->obj_type == OBJ_TYPE_MAP){
        struct spx_skp *tmp_skp = spx_skp_new_tmp(cmp_md, cmp_int, "tmp_skp", NULL, NULL);
        struct spx_skp_serial_md_list *tmp_mdl = NULL;

        //if obj->child != NULL, obj_type = OBJ_TYPE_MAP
        if (NULL != obj->child){
                msgpk_tree_query_exec(obj->child, tmp_skp);
        }

        if(*ov_key.str_val == '&'){
           tmp_mdl = msgpk_tree_merge_intersection(tmp_skp); 
        }
        else if(*ov_key.str_val == '|'){
           tmp_mdl = msgpk_tree_merge_union(tmp_skp); 
        } else{
            printf("no such query mark %s\n", ov_value.str_val);
            return;
        }
        
        msgpk_tree_fill_mdlist(parent_skp, tmp_mdl);
    } else{
        SpxSkpCmpDelegate cmp_key; 
        SpxSkpCmpDelegate cmp_value = cmp_md;
        SpxSkpB2ODelegate byte2key;
        void *v;
        switch(obj->obj_type){
            case OBJ_TYPE_STR://string
                cmp_key = cmp_str;
                byte2key = spx_skp_serial_byte2str;
                v = ov_value.str_val;
                break;
            case OBJ_TYPE_POSITIVE_INT:
            case OBJ_TYPE_NEGATIVE_INT:
            case OBJ_TYPE_INT32://int
                cmp_key = cmp_int;
                byte2key = spx_skp_serial_byte2int;
                v = &ov_value.int32_val;
                break;
            case OBJ_TYPE_INT64://long
                cmp_key = cmp_long;
                byte2key = spx_skp_serial_byte2long;
                v = &ov_value.int64_val;
                break;
            case OBJ_TYPE_FLOAT:
                cmp_key = cmp_float;
                byte2key = spx_skp_serial_byte2float;
                v = &ov_value.float_val;
            case OBJ_TYPE_DOUBLE:
                cmp_key = cmp_double;
                byte2key = spx_skp_serial_byte2double;
                v = &ov_value.double_val;
            default:
                v = NULL;
                printf("object_value type is not supported for msgpk_tree_query yet\n");
                return;
        }

        char key[MsgpkTreeKeySize];
        int op = msgpk_tree_parse_key(ov_key.str_val, key);        
        if(-1 == op){
            printf("msgpk_tree_parse_key error");
            return;
        }
        struct spx_skp *skp = spx_skp_get(key);
        if(NULL == skp){
            skp = spx_skp_add(cmp_key, cmp_value, byte2key, spx_skp_serial_byte2md, key, NULL, msgpk_tree_free_md); 
        }

        //spx_skp_print(skp);
        if(NULL == skp){
            printf("get_skp is NULL\n");
            return;
        }

        struct spx_skp_value_list *vl = NULL;

        if(0 == op){
            vl = spx_skp_query(skp, v);
        } else if (1 == op){
            vl = spx_skp_smaller_query(skp, v);
        } else if (2 == op){
            vl = spx_skp_bigger_query(skp, v);
        } else{
            printf("op is error\n");
            vl = NULL;
            return;
        }

        if(NULL != vl){
           msgpk_tree_fill_node_value(parent_skp, vl); 
           spx_skp_value_list_free(vl);
        }
    }

    if(NULL != obj->next){
        msgpk_tree_query_exec(obj->next, parent_skp);
    }
}/*}}}*/

struct spx_skp_serial_md_list *msgpk_tree_query(struct msgpk_object * obj){/*{{{*/
    if(NULL == obj->child){
        printf("obj->child is NULL\n");
        return NULL;
    }
    struct spx_skp *root_skp = spx_skp_new_tmp(cmp_md, cmp_int, "root_skp", NULL, NULL); 
    msgpk_tree_query_exec(obj->child, root_skp);
    return msgpk_tree_merge_union(root_skp); 
}/*}}}*/

char * msgpk_tree_add(struct msgpk_object *root, size_t req_size, char *request){/*{{{*/
    struct spx_skp_idx *tmp_idx = spx_skp_read_config(); 
    if (NULL == tmp_idx){
        printf("read index config error\n");
        return NULL;
    }

    struct spx_skp_serial_metadata *md = spx_skp_serial_data_writer2md((const ubyte_t*)request, req_size);
    char *unid = (char *) malloc(sizeof(char) * MsgpkTreeUnidSize);
    if(NULL == md){
        perror("data_writer2md:");
        return NULL;
    }

    spx_skp_serial_gen_unid(md, unid, MsgpkTreeUnidSize);

    while(tmp_idx->next_idx){
        tmp_idx = tmp_idx->next_idx;
        char *index = tmp_idx->index;
        SpxSkpCmpDelegate cmp_key; 
        SpxSkpCmpDelegate cmp_value = cmp_md;
        SpxSkpO2BDelegate key2byte;
        SpxSkpB2ODelegate byte2key;
        object_value kv;
        kv.str_val = index;
        object_type kt = OBJ_TYPE_STR;
        void *key = NULL;
        //SKP_TYPE skp_key_type = SKP_TYPE_STR;
        //SKP_TYPE skp_value_type = SKP_TYPE_MD;

        struct msgpk_object *obj = msgpk_tree_find_rule_node(root, 1, kt, kv);
        if (NULL == obj){
            printf("not found index: %s\n", index);
            continue;
        }

        if (0 == tmp_idx->type){
            cmp_key = cmp_str;
            key2byte = spx_skp_serial_str2byte;
            byte2key = spx_skp_serial_byte2str;
            key = (char *) calloc(1, sizeof(char) * (obj->obj_len + 1));
            strncpy(key, obj->value.str_val, obj->obj_len); 
            //skp_key_type = SKP_TYPE_STR;
        } else if (1 == tmp_idx->type){
            cmp_key = cmp_int;
            key2byte = spx_skp_serial_int2byte;
            byte2key = spx_skp_serial_byte2int;
            key = (int *) malloc(sizeof(int));
            *(int *)key = obj->value.int32_val;
            //skp_key_type = SKP_TYPE_INT;
        } else if (2 == tmp_idx->type){
            cmp_key = cmp_long;
            key2byte = spx_skp_serial_long2byte;
            byte2key = spx_skp_serial_byte2long;
            key = (int64_t *) malloc(sizeof(int64_t));
            *(int64_t*)key = obj->value.int64_val;
            //skp_key_type = SKP_TYPE_LONG;
        } else {
            printf("not support type of index\n");
            continue;
        }


        struct spx_skp *skp = spx_skp_get(index);
        if(NULL == skp){
            skp = spx_skp_add(cmp_key, cmp_value, byte2key, spx_skp_serial_byte2md, index, NULL, msgpk_tree_free_md); 
        }

        //spx_skp_print(skp);
        spx_skp_insert(skp, key, md); 
        //spx_skp_print(skp);
        spx_skp_serial(skp, key2byte, spx_skp_serial_md2byte);
    }

    return unid;
}/*}}}*/
