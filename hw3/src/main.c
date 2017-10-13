#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

/*

    double* ptr = sf_malloc(sizeof(double));
    double* ptr2 = sf_malloc(32);
    double* ptr3 = sf_malloc(1000);
    double* ptr4 = sf_realloc(ptr, 2800);
    double* ptr5 = sf_malloc(5000);
    double* ptr6 = sf_realloc(ptr3, 500);
    double* ptr7 = sf_malloc(200);
    double* ptr8 = sf_malloc(240);
//    double* ptr9 = sf_malloc(8000);


//    *ptr = 320320320e-320;

//    printf("%f\n", *ptr);

//    sf_free(ptr);
    sf_free(ptr2);
//    sf_free(ptr3);
    sf_free(ptr4);
    sf_free(ptr5);
    sf_free(ptr6);
    sf_free(ptr7);
    sf_free(ptr8);
//    sf_free(ptr9);
*/

    /* void *u = */ sf_malloc(1);          //32
    void *v = sf_malloc(LIST_1_MIN); //48
    void *w = sf_malloc(LIST_2_MIN); //160
    void *x = sf_malloc(LIST_3_MIN); //544
    void *y = sf_malloc(LIST_4_MIN); //2080
    /* void *z = */ sf_malloc(1); // 32

//    int allocated_block_size[4] = {48, 160, 544, 2080};

    sf_free(v);
    sf_free(w);
    sf_free(x);
    sf_free(y);


    sf_mem_fini();

    return EXIT_SUCCESS;
}
