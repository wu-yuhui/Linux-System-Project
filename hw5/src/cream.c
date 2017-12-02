#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "stdio.h"
#include "csapp.h"
//#include "sbuf.h"

queue_t *requestQueue;
hashmap_t *myMap;


//void destroy_function(void *);
void hash_destroy_function(map_key_t, map_val_t);
void display_help(){
    printf("./cream [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES \n");
    printf("-h                 Displays this help menu and returns EXIT_SUCCESS. \n");
    printf("NUM_WORKERS        The number of worker threads used to service requests. \n");
    printf("PORT_NUMBER        Port number to listen on for incoming connections. \n");
    printf("MAX_ENTRIES        The maximum number of entries that can be stored in `cream`'s underlying data store.\n");
    return;
}
void *thread(void *);
bool check_key_size(uint32_t);
bool check_value_size(uint32_t);

response_header_t bad_request_error(){
    response_header_t response_header;
    response_header.response_code = BAD_REQUEST;
    response_header.value_size = 0;
    return response_header;
}

response_header_t not_found_error(){
    response_header_t response_header;
    response_header.response_code = NOT_FOUND;
    response_header.value_size = 0;
    return response_header;
}

response_header_t unsupported_error(){
    response_header_t response_header;
    response_header.response_code = UNSUPPORTED;
    response_header.value_size = 0;
    return response_header;
}
response_header_t assign_response(uint32_t response_code, uint32_t value_size){
    response_header_t response_header;
    response_header.response_code = response_code;
    response_header.value_size = value_size;
    return response_header;
}


int main(int argc, char *argv[]) {

    //int connfd;
    socklen_t clientlen;
    SA clientaddr;
    int listenfd;

    // Check prompt input vaildation
    char h[3] = "-h";
    if (!strcmp(argv[1], h)){
        display_help();
        return EXIT_SUCCESS;
    }
    else if (argc != 4){
        fprintf(stderr, "EEROR:invalid number of arguments.\nUsage:\n");
        display_help();
        return EXIT_FAILURE;
    }

    requestQueue = create_queue();
    if (requestQueue == NULL){
        fprintf(stderr, "%s\n", "Create  request queue fail");
        return EXIT_FAILURE;

    }
    myMap = create_map(atoi(argv[3]), jenkins_one_at_a_time_hash, hash_destroy_function);  // Handle hash Function & Destroy

    // Spawn threads
    for (int t_count = 0; t_count < atoi(argv[1]); t_count++){
        pthread_t tid;
        Pthread_create (&tid, NULL, thread, NULL);
    }
    listenfd = Open_listenfd(argv[2]);
    //printf("listenfd: %d", listenfd);
    while(1){
        clientlen = sizeof(clientaddr);
        int *connfd = calloc(1, sizeof(int));
        *connfd = Accept(listenfd, &clientaddr, &clientlen);
        //printf("Connfd in main thread:%d\n", *connfd);
        if (!enqueue(requestQueue, connfd)){
            fprintf(stderr, "%s\n", "Failure in enqueue");
            return EXIT_FAILURE;
        }
    }

/*  // test
    queue_t *tmp;

    if ((tmp = create_queue()) == NULL)
        printf("NULL\n");
    else
        printf("Create OK\n");

    int arr[] = {3,5,2,5,6};

    for (int i = 0; i < 5; i++){
        int *a = malloc(sizeof(int));
        *a = arr[i];
        enqueue(tmp, a);
    }

    for (int j = 0; j < 5; j++)
        dequeue(tmp);

    invalidate_queue(tmp, destroy_function);
*/
    //return EXIT_SUCCESS;
    exit(0);
}

void *thread(void *vargp){

    while(1){
        // TAKE FD to do stuff
        int *connfd = dequeue(requestQueue);
        if (connfd == NULL){
            fprintf(stderr, "%s\n", "Failure in enqueue1");
            exit(EXIT_FAILURE);
        }
        // Read and do work
        request_header_t *request_header = calloc(1, sizeof(request_header_t));
        response_header_t *response_header = calloc(1, sizeof(response_header_t));
        //printf("Connfd in thread:%d\n", *connfd);
        Rio_readn(*connfd, request_header, sizeof(request_header_t));

        //printf("Read from fd: %d, request_code: %u, key_size:%u, val_size:%u\n", *connfd, request_header-> request_code, request_header->key_size, request_header->value_size );

        //map_key_t *key = calloc(1, sizeof(map_key_t));
        //map_val_t *val = calloc(1, sizeof(map_val_t));

        if (request_header->request_code == PUT){

            if (!check_key_size(request_header->key_size) || !check_value_size(request_header->value_size)){
                *response_header =  bad_request_error();              // key or value size error
            }
            else{

                void *key_base = calloc(1, sizeof(request_header->key_size));
                void *val_base = calloc(1, sizeof(request_header->value_size));

                Rio_readn(*connfd, key_base, request_header->key_size);
                Rio_readn(*connfd, val_base, request_header->value_size);

                if (!put(myMap, MAP_KEY(key_base, request_header->key_size), MAP_VAL(val_base, request_header->value_size), 1)) {
                    *response_header =  bad_request_error();          // PUT Hashmap ERROR
                }
                else{
                    *response_header = assign_response(OK, request_header->value_size);
                }
            }

            Rio_writen(*connfd, response_header, sizeof(response_header_t));
            close(*connfd);

        }
        else if (request_header->request_code == GET){

            if (!check_key_size(request_header->key_size)){
                *response_header =  bad_request_error();              // key or value size error
                Rio_writen(*connfd, response_header, sizeof(response_header_t));
            }
            else{
                void *key_base = calloc(1, sizeof(request_header->key_size));
                void *val_base = calloc(1, sizeof(request_header->value_size));
                Rio_readn(*connfd, key_base, request_header->key_size);

                size_t val_len = 0;
                val_base = get(myMap, MAP_KEY(key_base, request_header->key_size)).val_base;
                val_len = get(myMap, MAP_KEY(key_base, request_header->key_size)).val_len;

                //printf("key_base:%p \n", key_base);
                //printf("val_base:%p  val_len: %lu\n", val_base, val_len);

                if (val_base == NULL || val_len == 0) {
                    *response_header =  not_found_error();          // PUT Hashmap ERROR
                }
                else{
                    *response_header = assign_response(OK, val_len);
                }

                Rio_writen(*connfd, response_header, sizeof(response_header_t));
                if (response_header->response_code == OK && response_header->value_size != 0)
                    Rio_writen(*connfd, val_base, val_len);
            }


            close(*connfd);


        }
        else if (request_header->request_code == EVICT){

            if (!check_key_size(request_header->key_size)){
                *response_header =  assign_response(OK, 0);
            }
            else{
                void *key_base = calloc(1, sizeof(request_header->key_size));
                Rio_readn(*connfd, key_base, request_header->key_size);

                delete(myMap, MAP_KEY(key_base, request_header->key_size));
                *response_header = assign_response(OK, 0);
            }

            Rio_writen(*connfd, response_header, sizeof(response_header_t));
            close(*connfd);

        }
        else if (request_header->request_code == CLEAR){
            clear_map(myMap);
            *response_header = assign_response(OK, 0);
            Rio_writen(*connfd, response_header, sizeof(response_header_t));
            close(*connfd);

        }
        else{
            // Invalid
            *response_header = unsupported_error();
            Rio_writen(*connfd, response_header, sizeof(response_header_t));
            close(*connfd);
        }

    }
}




/*
void destroy_function(void *item){
    free(item);
}
*/

void hash_destroy_function(map_key_t key, map_val_t val) {
    free(key.key_base);
    free(val.val_base);
}

bool check_key_size(uint32_t keySize){
    if (keySize >= MIN_KEY_SIZE && keySize <= MAX_KEY_SIZE)
        return true;
    else
        return false;
}
bool check_value_size(uint32_t valueSize){
    if (valueSize >= MIN_VALUE_SIZE && valueSize <= MAX_VALUE_SIZE)
        return true;
    else
        return false;
}