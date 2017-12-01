#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "stdio.h"
#include "csapp.h"
//#include "sbuf.h"

void destroy_function(void *);
void display_help(){
    printf("./cream [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES \n");
    printf("-h                 Displays this help menu and returns EXIT_SUCCESS. \n");
    printf("NUM_WORKERS        The number of worker threads used to service requests. \n");
    printf("PORT_NUMBER        Port number to listen on for incoming connections. \n");
    printf("MAX_ENTRIES        The maximum number of entries that can be stored in `cream`'s underlying data store.\n");
    return;
}

int main(int argc, char *argv[]) {

//    int i, listenfd, connfd;
//    socklen_t clientlen;
//    struct sockaddr_storage clientaddr;
//    pthread_t tid;

    char h[3] = "-h";
    if (!strcmp(argv[1], h)){
        display_help();
        return EXIT_SUCCESS;
    }
    else if (argc != 4){
        app_error("invalid number of arguments.");
        return EXIT_FAILURE;
    }

    listenfd = Open_listenfd(argv[2]);



/* Test
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

/*%s
void destroy_function(void *item){
    free(item);
}
*/

