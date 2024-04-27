#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "lib.h"

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        perror("Número de argumentos inválido!\n");
        return 1;
    }
    if (strcasecmp(argv[1], "execute") == 0)
    {
        if (argc != 5)
        {
            perror("Número de argumentos inválido no execute\n");
            return 1;
        }

        Program program;
        create_program(&program, argv);
        printf("Time: %d\t Flag: %s\t Arguments: %s\n", program.time, program.flag, program.arguments);
    }
    else if (strcasecmp(argv[1], "status") == 0)
    {
        printf("hello from status\n");
    }
    else
    {
        perror("Opção incorreta!\n");
        return 1;
    }

    return 0;
}
