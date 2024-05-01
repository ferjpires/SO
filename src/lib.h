#ifndef LIB_H
#define LIB_H

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct
{
    int running; // 1 for running, 0 for done
    int status; // 1 for status, 0 for execute
    int time;
    long time_s;
    long time_ms;
    char flag[3];
    char arguments[300];
    pid_t processID;
} Program;

#define MAX 100

typedef struct queue 
{
    Program values[MAX] ;
    int inicio , tamanho ;
} Queue ;

typedef struct
{
    Program *executing;
    Queue in_queue[100];
    Program *finished;
} ToUser;

void create_program(Program *program, char const *argv[]);

void parseArguments(Program program, char *exec_args[]);

#endif
