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

typedef struct finished
{
    Program values[1000];
    int tamanho ;
} Finished ;

typedef struct
{
    Program *executing;
    Queue in_queue;
    Finished finished;
} ToUser;

void create_program(Program *program, char const *argv[]);

void parseArguments(Program program, char *exec_args[]);

void initQueue (Queue *q);

int isEmpty(Queue *queue);

int isFull(Queue *queue);

void enqueue(Queue *queue, Program value);

void dequeue (Queue *queue);

void createToUser(ToUser *touser, int parallel);

int is_there_space(ToUser *touser,int parallel);

#endif
