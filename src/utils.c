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