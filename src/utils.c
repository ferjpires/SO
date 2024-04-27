#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "lib.h"


void create_program(Program *program, char const *argv[])
{
    program->time = atoi(argv[2]);
    strcpy(program->flag,argv[3]);
    strcpy(program->arguments, argv[4]);
}

void parseArguments(Program program, char *exec_args[])
{
    char *args;
    args = strdup(program.arguments);
    int i = 0;
    char *string;
    string = strsep(&args, " ");
    while(string != NULL){
        exec_args[i] = string;
        string = strsep(&args," ");
        i++;
    }
    exec_args[i] = NULL;
}