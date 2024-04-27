#ifndef LIB_H
#define LIB_H

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct
{
    int running; // 1 for running, 0 for done
    int time;
    char flag[3];
    char arguments[300];
    pid_t processID;
} Program;

void create_program(Program *program, char const *argv[]);

#endif