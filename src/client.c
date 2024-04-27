#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>


#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

#include "lib.h"

int main(int argc, char const *argv[])
{

    const char *fifoPath = "tmp/my_fifo"; // Path to the FIFO

    int fd = open(fifoPath, O_WRONLY);

    if (argc < 2)
    {
        perror("Número de argumentos inválido!\n");
        return 1;
    }
    if (strcasecmp(argv[1], "execute") == 0)
    {
        if (argc != 5)
        {
            perror("Número de argumentos inválido no execute!\n");
            return 1;
        }
        if (atoi(argv[2]) < 1)
        {
            perror("Tempo (ms) não pode ser inferior a 1!\n");
            return 1;
        }
        
        Program program;
        create_program(&program, argv);
        program.processID = getpid();

        write(fd,&program,sizeof(program));

        // i don't know what this is for ??? 

        // struct timeval time;
        // gettimeofday(&time, NULL);
        // program.time_ms = time.tv_usec;
        // program.time_s = time.tv_sec;
        // tam = snprintf(output, sizeof(output), "#%d#%d#%d#%ld#%ld#%s#", program.processID, 0, 0, program.time_s, program.time_ms, program.arguments);
        // write(1, output, tam); //aqui tinha fd

        //printf("Time: %d\t Flag: %s\t Arguments: %s\n", program.time, program.flag, program.arguments);
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
