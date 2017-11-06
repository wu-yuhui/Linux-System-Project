#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sfish.h"
#include "debug.h"

#define BUILTIN_NUM 1
#define MAXARGS 128
#define MAXLINE 256

char old_path[PATH_MAX] = {};


/* Evaluate a command line */
void eval(char *cmdline, char** envp){
    char *argv[MAXARGS] = {};    /* Argument list execve() */
    char buf[MAXLINE];      /* Holds modified command line */
    // int bg;                 /* background or foreground */
    pid_t pid;

    strcpy(buf, cmdline);   /* Manipulate buf, prevent modifying the input */
    //bg = parseline(buf, argv);

    if (how_much_redirect(cmdline)){
        printf(SYNTAX_ERROR, "Redirection too much");
        return;
    }

    // See which redir_case
    int redir_case = 0;
    char *p = buf;
    int already_in = 0;
    for (int i = 0; i < strlen(buf); i++){
        if (*(p+i) == '<'){
            redir_case++;
            already_in++;
        }
        if (*(p+i) == '>'){
            if (already_in) redir_case += 3;
            else    redir_case += 2;
        }
    }
    /*
    <:  1
    >:  2
    ><: 3
    <>: 4
    */

    char in_target[MAXLINE] = {};
    char out_target[MAXLINE] = {};


    switch(redir_case){
        case 1:     // <
            redir_case1(buf, argv, in_target);
            break;
        case 2:     // >
            redir_case2(buf, argv, out_target);
            break;
        case 3:     // ><
            redir_case3(buf, argv, in_target, out_target);
            break;
        case 4:     // <>
            redir_case4(buf, argv, in_target, out_target);
            break;
        default:
            parseline(buf, argv);
            break;
    }

/*    for (int i = 0; argv[i] != NULL; i++){
        printf("argv[%d]: %s\n", i, argv[i]);
    }
*/

    if (argv[0] == NULL){
        printf("argv[0] == NULL\n");
        return;
    }

    if(builtin_command(argv) == -1){
        // Other than built-in commands

        // run
        if ((pid = fork()) == 0){
            //printf("Child pid is %d\n", getpid());
        //////////////////////////////////////  setmask

            if (strlen(in_target)){  // redir in target exist
                int in;
                if((in = open(in_target, O_RDONLY)) < 0){
                    perror("read");
                    exit(1);
                }
                dup2(in, STDIN_FILENO);
                close(in);

            }
            if (strlen(out_target)){ // redir out target exist
                int out;
                if((out = open(out_target, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR)) < 0){
                    perror("write");
                    exit(1);
                }
                dup2(out, STDOUT_FILENO);
                close(out);
            }
           // Child run user-job
            if (execvp(argv[0], argv) < 0){
                printf(EXEC_NOT_FOUND, argv[0]);
                //exit(0);
            }
        }
        //Parent
        ///////////////////////////    tcsetgrp
        ///////////////////////////    sigsuspend
        int status;
        if(waitpid(pid, &status, WUNTRACED) < 0)
            if (WIFEXITED(status))
                printf("Child %d terminated with exit status %d\n", pid, WEXITSTATUS(status));

        //printf("Parent pid is %d\n", getpid());

    }

/*
    for(int i = 0; argv[i] != NULL; i++){
        printf("printing argv[%d]: %s\n", i, argv[i]);
    }
*/


    return;

}


void parseline(char *buf, char **argv){

    char *delim;        /* Points to first space delimiter */
    int argc;           /* Number of args */
//    int bg = 0;             /* Background jod */

    buf[strlen(buf)] = ' ';
    while(*buf && (*buf == ' '))    buf++;


    /* Build the argv list */
    argc = 0;
    while((delim = strchr(buf, ' '))){
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while(*buf && (*buf == ' '))    buf++;
    }
    argv[argc] = NULL;

    /*
    if(argc == 0){
        printf("Input is NULL!!\n");
        return 1;
    }
    */

    /* Should the job run in the background? */
    /*
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;
    */

    //return bg;


}
int builtin_command(char** input){
    /*  1: SUCCESS
        0: FAIL
        -1: NOT FOUND
    */

    if (!strcmp(input[0], "help"))
        print_help();
    else if (!strcmp(input[0], "exit"))
        exit(0);
    else if (!strcmp(input[0], "cd")){
        if (input[1] == NULL){
            char *home[2] = {};
            home[0] = input[0];
            char *home_environ;
            if ((home_environ = getenv("HOME")) == NULL){
                printf(BUILTIN_ERROR, input[0]);
                return 0;
            }
            printf("HOME: %s\n", home_environ);
            home[1] = home_environ;

            if (!change_to_path(home))
                return 0;
        }
        else if (!strcmp(input[1], "-")){

            if (strlen(old_path) == 0){
                printf(BUILTIN_ERROR, "cd - don't have old path");
                return 0;
            }

            if (!change_to_old_path())
                return 0;

        }
        else {
            if (!change_to_path(input))
                return 0;
        }
    }
    else if (!strcmp(input[0], "pwd")){    //getcwd(3)
        char currentPath[PATH_MAX] = {};
        if (getcwd(currentPath, PATH_MAX) == NULL){
            printf(BUILTIN_ERROR, input[0]);
            return 0;
        }
        else printf("%s\n", currentPath);
    }
    else
        return -1;

    return 1;
}


void print_help(){
    printf("=====\n");
    printf("These shell commands are defined internally.  Type `help' to see this list.\n");
    printf(" cd [dir]:  change to specific directory\n");
    printf(" exit:      terminate program and exit\n");
    printf(" help:      print help manual\n");
    printf(" pwd:       current directory path\n");
    printf("=====\n");
}

int change_to_old_path(){
    char currentPath[PATH_MAX] ={};
    if (getcwd(currentPath, PATH_MAX) == NULL){
        printf(BUILTIN_ERROR, "cd: cannot find current path");
        return 0;
    }
    char target_path[PATH_MAX] = {};
    memcpy(target_path, old_path, strlen(old_path));
    memset(old_path, '\0', strlen(old_path));
    memcpy(old_path, currentPath, strlen(currentPath));
    chdir(target_path);
    return 1;
}

int change_to_path(char **input){

       // Get current path
    char currentPath[PATH_MAX] ={};
    if (getcwd(currentPath, PATH_MAX) == NULL){
        printf(BUILTIN_ERROR, "cd: cannot find current path");
        return 0;
    }

    // If chdir OK, update old_path
    if (chdir(input[1]) < 0){
        printf(BUILTIN_ERROR, input[0]);
        return 0;
    }
    else{
        memset(old_path, '\0', strlen(old_path));
        memcpy(old_path, currentPath, strlen(currentPath));
    }

    return 1;
}

int how_much_redirect(char *cmdline){
    char *ptr = cmdline;
    int in = 0;
    int out = 0;

    int count = 0;
    int cmdlen = strlen(cmdline);

    while (count < cmdlen){
        if (*(ptr+count) == '<')
            in++;
        else if (*(ptr+count) == '>')
            out++;
        count++;
    }

    if (in > 1 || out > 1)
        return 1;
    else return 0;
}


void redir_case1(char *buf, char **argv, char *in_target){
    char *p = buf;
    for (int i = 0; i < strlen(buf); i++){
        if (*(p+i) == '<')
            *(p+i) = ' ';
    }
    parseline(buf, argv);

    int argvLength = 0;
    while(argv[argvLength] != NULL)
        argvLength++;
    // STDiN to argv[argvLength-1];

    strcpy(in_target, argv[argvLength-1]);
    argv[argvLength-1] = NULL;

}

void redir_case2(char *buf, char **argv, char *out_target){
    char *p = buf;
    for (int i = 0; i < strlen(buf); i++){
        if (*(p+i) == '>')
            *(p+i) = ' ';
    }
    parseline(buf, argv);

    int argvLength = 0;
    while(argv[argvLength] != NULL)
        argvLength++;
    // STDiN to argv[argvLength-1];

    strcpy(out_target, argv[argvLength-1]);
    argv[argvLength-1] = NULL;
}


void redir_case3(char *buf, char **argv, char *in_target, char * out_target){
    char *p = buf;
    for (int i = 0; i < strlen(buf); i++){
        if (*(p+i) == '>' || *(p+i) == '<')
            *(p+i) = ' ';
    }
    parseline(buf, argv);

    int argvLength = 0;
    while(argv[argvLength] != NULL)
        argvLength++;
    // STDiN to argv[argvLength-1];
    strcpy(in_target, argv[argvLength-1]);
    strcpy(out_target, argv[argvLength-2]);

    argv[argvLength-1] = NULL;
    argv[argvLength-2] = NULL;
}

void redir_case4(char *buf, char **argv, char *in_target, char * out_target){
    char *p = buf;
    for (int i = 0; i < strlen(buf); i++){
        if (*(p+i) == '>' || *(p+i) == '<')
            *(p+i) = ' ';
    }
    parseline(buf, argv);

    int argvLength = 0;
    while(argv[argvLength] != NULL)
        argvLength++;
    // STDiN to argv[argvLength-1];
    strcpy(in_target, argv[argvLength-2]);
    strcpy(out_target, argv[argvLength-1]);


    argv[argvLength-1] = NULL;
    argv[argvLength-2] = NULL;
}
