#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "stdio.h"

void destroy_function(void *);

int main(int argc, char *argv[]) {
    //comment

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


    exit(0);
}

void destroy_function(void *item){
    free(item);
}