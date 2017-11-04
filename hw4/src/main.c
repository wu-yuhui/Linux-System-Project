#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>

#include "sfish.h"
#include "debug.h"

int main(int argc, char *argv[], char* envp[]) {
    char* input;
    bool exited = false;

    if(!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }

    do {

        input = readline("> ");


        // If EOF is read (aka ^D) readline returns NULL
        if(input == NULL) {
            continue;
        }


        /*
        write(1, "\e[s", strlen("\e[s"));
        write(1, "\e[20;10H", strlen("\e[20;10H"));
        write(1, "SomeText", strlen("SomeText"));
        write(1, "\e[u", strlen("\e[u"));
        */
        char manInput[256];
        strcpy(manInput, input);

        printf("The manInput: %s\n", manInput);

        // Parsing Input
        char inputArray[32][128] = {};
        char *p = strtok(input, " ");
        int i = 0;
        while(p != NULL) {
            memcpy(inputArray[i], p, strlen(p));
            // printf("memcpy of Input Array Elements [%d] %s DONE\n", i, p);
            p = strtok(NULL, " ");
            i++;
        }

        // Check if not builtIn command -> skip iteration
        if (not_builtin(inputArray[0])){
            printf(BUILTIN_ERROR, inputArray[0]);
            continue;
        }


        // Currently nothing is implemented
        printf(EXEC_NOT_FOUND, input);

        // You should change exit to a "builtin" for your hw.
        exited = strcmp(input, "exit") == 0;

        // Readline mallocs the space for input. You must free it.
        rl_free(input);

    } while(!exited);

    debug("%s", "user entered 'exit'");

    return EXIT_SUCCESS;
}
