/*************************************************************************
    > File Name: client.c
    > Author: shuaixiang
    > Mail: shuaixiang@yuewen.com
    > Created Time: Thu 24 Nov 2016 04:05:10 PM CST
************************************************************************/
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "msgpk_build_tree.h"
#include "msgpk_tree.h"

#include "messagepack.h"
#include "skiplist.h"
#include "spx_log.h"
#include "thread_pool.h"
#include "logdb_debug.h"

#define ThreadCount 1
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int rand_int(int base){
    int val = rand() % base;
    return val;
}

static char* rand_str(int len){
    char* str = (char*) calloc(1, sizeof(char) * (len + 1)); 
    int i;
    for (i = 0; i < len; i++){
        *(str + i) = 'a' + rand_int(26);
    }

    return str;
}

int total = 0;
void insert_test(void* q){
    int count = *(int*)q;
    const char* request = {"111111111"};
    int i;
    for (i = 0; i < count; i++){
        struct tree_context* ctx = msgpk_build_init();

        ////age
        //msgpk_build_map_string_int(ctx, "age", 3, rand_int(120));

        //name
        char *name = rand_str(7);
        printf("name =================================>%s\n", name);
        msgpk_build_map_string_string(ctx, "name", 4, name, strlen(name));

        ////city
        //char *city = rand_str(7);
        //msgpk_build_map_string_string(ctx, "city", 4, city, strlen(city));

        ////hobbies
        //char *hobbies = rand_str(4);
        //msgpk_build_map_string_string(ctx, "hobbies", 7, hobbies, strlen(hobbies));

        ////idol
        //char *idol = rand_str(3);
        //msgpk_build_map_string_string(ctx, "idol", 4, idol, strlen(idol));

        ////food
        //char *food = rand_str(6);
        //msgpk_build_map_string_string(ctx, "food", 4, food, strlen(food));

        ////father
        //char *father = rand_str(4);
        //msgpk_build_map_string_string(ctx, "father", 6, father, strlen(father));

        ////mother
        //char *mother = rand_str(4);
        //msgpk_build_map_string_string(ctx, "mother", 6, mother, strlen(mother));

        ////spouse
        //char *spouse = rand_str(4);
        //msgpk_build_map_string_string(ctx, "spouse", 6, spouse, strlen(spouse));

        ////animal
        //char *animal = rand_str(3);
        //msgpk_build_map_string_string(ctx, "animal", 6, animal, strlen(animal));

        ////plant
        //char *plant = rand_str(4);
        //msgpk_build_map_string_string(ctx, "plant", 5, plant, strlen(plant));

        ////weixin
        //char *weixin = rand_str(8);
        //msgpk_build_map_string_string(ctx, "weixin", 6, weixin, strlen(weixin));

        ////sex
        //char sex[1]; 
        //sex[0] = '0' + rand_int(2);
        //msgpk_build_map_string_string(ctx, "sex", 3, sex, 1);

        ////address
        //char *addr = rand_str(9);
        //msgpk_build_map_string_string(ctx, "address", 7, addr, strlen(addr));

        ////score
        //msgpk_build_map_string_int(ctx, "score", 5, rand_int(100));

        
        //msgpk_tree_print_json(ctx->root);
        //printf("\n");
        //struct msgpk_object* obj = msgpk_tree_find_rule_node(ctx->root, 1, OBJ_TYPE_STR, "age"); 
        //printf("age:%d\n", obj->value.int32_val);
        
        pthread_mutex_lock(&mutex);
        logdb_debug("thread:%ld ======================================>%d\n", pthread_self(), total++);
        char* unid = msgpk_tree_add(ctx->root, strlen(request), (char*)request);
        pthread_mutex_unlock(&mutex);
        free(unid);
        msgpk_build_free(ctx);
    }
}

int main(){
    //init
    SpxLogDelegate* log = spx_log;
    //    printf("%d.\n",getpid());
    err_t err = 0;
    pthread_t block_tid = spx_block_skp_thread_new(log, &err);
    if (err != 0){
        SpxLog2(log, SpxLogError, err, "spx_skp_serial_block_skp_thread_new failed");
        exit(err);
    }

    spx_block_skp_config_task_queue_init();
    spx_block_skp_config_pool_init();
    spx_block_skp_config_task_pool_init();
    //thread_pool_init(spx_block_skp_count_config_index());
    thread_pool_init(1);

    //------------------------------------------------
    clock_t start;
    clock_t end;
    pthread_t tid[ThreadCount];

    int count = 5000000;
    int i;
    srand((int) time(0));

    start = clock();
    for(i = 0; i < ThreadCount; i++){
        if (pthread_create(&tid[i], NULL, (void*)insert_test, &count) != 0){
            logdb_debug("pthread_create failed\n");
        }
        logdb_debug("------------------------------------------------->thread:%d start to  work\n", i);
    }


    for(i = 0; i < ThreadCount; i++)
        pthread_join(tid[i], NULL);

    end = clock();

    double cost = (double)(end - start)/CLOCKS_PER_SEC;
    //pthread_join(block_tid, NULL);
    //printf("======================================\n");
    //printf("add %d cost time: %dms \n", count, (int)(cost * 1000));
    
    return 0;
}

