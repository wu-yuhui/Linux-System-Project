#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
    //comment

    queue_t *tmp;

    if ((tmp = create_queue()) == NULL)
        printf("NULL\n");
    else
        printf("Create OK\n");

    int arr[] = {0, 1, 2, 3, 4};

    for (int i = 0; i < 5; i++)
        enqueue(tmp, arr+i);
    for (int j = 0; j < 5; j++)
        dequeue(tmp);

    exit(0);
}
