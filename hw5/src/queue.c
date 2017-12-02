#include "queue.h"
#include <errno.h>
#include <stdio.h>

void print_queue(queue_t *self){
    if (self->front == NULL){
        printf("{}\n");
        return;
    }

    queue_node_t *ptr = self->front;

    do{
        printf("%d, ", *(int *)ptr->item);
        ptr = ptr -> next;
    }while(ptr != NULL);

    printf("\n");

    return;
}


void destroy_function(void *item){
    free(item);
}


queue_t *create_queue(void) {

    queue_t *myQueue = calloc(1, sizeof(queue_t));
    if (myQueue == NULL)    return NULL;

    myQueue->front = NULL;
    myQueue->rear = NULL;

    if (sem_init(&myQueue->items, 0, 0) != 0)    //initialize queue but not mutexes inside
        return NULL;                            // Errno?
    if (pthread_mutex_init(&myQueue->lock, NULL) != 0)
        return NULL;                               // Errno?

    myQueue->invalid = 0;

    // print_queue(myQueue);

    return myQueue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {

    if (self == NULL){
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->lock);

    if (self->invalid){
        errno = EINVAL;
        pthread_mutex_unlock(&self->lock);
        return false;
    }

    queue_node_t *ptr = self->front;
    while(ptr != NULL){
        destroy_function(ptr->item);
        queue_node_t *toFree = ptr;
        ptr = ptr->next;
        free(toFree);
        //print_queue(self);
    }
    self->invalid = 1;

    pthread_mutex_unlock(&self->lock);

    return true;
}

bool enqueue(queue_t *self, void *item) {

    if (self == NULL || item == NULL){
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->lock);

    if (self->invalid){
        errno = EINVAL;
        pthread_mutex_unlock(&self->lock);
        return false;
    }

    queue_node_t *thisNode = calloc(1, sizeof(queue_node_t));
    /////////// if (thisNode == NULL)    return false;

    thisNode->item = item;

    if (self->rear == NULL){        // Empty queue
        self->front = thisNode;
        self->rear = thisNode;
        thisNode->next = NULL;
    }
    else{                           // Queue with somehting
        self->rear->next = thisNode;
        self->rear = thisNode;
        thisNode->next = NULL;
    }

    if (sem_post(&self->items)){     // Semaphore++
        errno = EINVAL;
        pthread_mutex_unlock(&self->lock);
        return false;
    }

    // print_queue(self);
    pthread_mutex_unlock(&self->lock);
    return true;
}

void *dequeue(queue_t *self) {

    if (self == NULL){
        errno = EINVAL;
        printf("%s\n", "1");
        return NULL;
    }

    pthread_mutex_lock(&self->lock);

    if (self->invalid){
        errno = EINVAL;
        pthread_mutex_unlock(&self->lock);
        return NULL;
    }

    pthread_mutex_unlock(&self->lock);

    if (sem_wait(&self->items)){
        errno = EINVAL;
        return NULL;
    }

    pthread_mutex_lock(&self->lock);

    queue_node_t *thisNode = self->front;
    void *item = thisNode -> item;
    if (thisNode == self->rear){
        self->front = NULL;
        self->rear = NULL;
    }
    else
        self->front = self->front->next;

    free(thisNode);

    // print_queue(self);
    pthread_mutex_unlock(&self->lock);
    return item;
}