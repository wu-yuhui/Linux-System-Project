#ifndef SFISH_H
#define SFISH_H

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_ERROR  "sfish builtin error: %s\n"
#define SYNTAX_ERROR   "sfish syntax error: %s\n"
#define EXEC_ERROR     "sfish exec error: %s\n"

#endif

//int run_command(char*);
//int builtin(char*);
void eval(char*, char**);
void parseline(char *, char **);
int builtin_command(char**);
void print_help();
int change_to_old_path();
int change_to_path(char**);
int how_much_redirect(char *);
void redir_case1(char *, char **, char *);
void redir_case2(char *, char **, char *);
void redir_case3(char *, char **, char *, char *);
void redir_case4(char *, char **, char *, char *);