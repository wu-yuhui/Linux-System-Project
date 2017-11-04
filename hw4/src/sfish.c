#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>

#include "sfish.h"
#include "debug.h"

#define BUILTIN_NUM 4

int run_command(char* input){
    if (not_builtin(input) == 1)
        return 1;
    // If builtIn fail :standard out saying sfish builtin error: %s\n. BUILTIN_ERROR








    return 0;
}

int not_builtin(char* input){

    const char* builtin[BUILTIN_NUM] = {"help", "exit", "cd", "pwd"};

    for(int i = 0; i < BUILTIN_NUM; i++){
        printf("BuiltIn:%s, Input: %s\n", builtin[i], input);
        if (strcmp(input, builtin[i]) == 0)     return 0;
    }

    return 1;

}