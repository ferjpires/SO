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

    const char *fifoPath = "tmp/my_fifo";     // Path to the FIFO
    const char *pidFifoPath = "tmp/pid_fifo"; // Path to the process ID FIFO
    int errors = open("src/errors.txt", O_RDWR | O_CREAT | O_APPEND, 0666);

    int fd = open(fifoPath, O_WRONLY);
    if (fd == -1)
    {
        perror("Error opening in client\n");
        write(errors,"Error opening in client\n",sizeof("Error opening in client\n"));
        return 1;
    }
    int fd2 = open(pidFifoPath, O_RDONLY);
    if (fd2 == -1)
    {
        perror("Error opening pid Fifo in client\n");
        write(errors,"Error opening pid Fifo in client\n",sizeof("Error opening pid Fifo in client\n"));
        return 1;
    }

    if (argc < 2)
    {
        perror("Número de argumentos inválido!\n");
        write(errors,"Número de argumentos inválido!\n",sizeof("Número de argumentos inválido!\n"));
        return 1;
    }
    if (strcasecmp(argv[1], "execute") == 0)
    {
        if (argc != 5)
        {
            perror("Número de argumentos inválido no execute!\n");
            write(errors,"Número de argumentos inválido no execute!\n",sizeof("Número de argumentos inválido no execute!\n"));
            return 1;
        }
        if (atoi(argv[2]) < 1)
        {
            perror("Tempo (ms) não pode ser inferior a 1!\n");
            write(errors,"Tempo (ms) não pode ser inferior a 1!\n",sizeof("Tempo (ms) não pode ser inferior a 1!\n"));
            return 1;
        }

        Program program;
        create_program(&program, argv);
        program.processID = getpid();

        write(fd, &program, sizeof(program));

        int pid;
        if (read(fd2, &pid, sizeof(int)) > 0)
        {
            printf("o pid é: %d\n", pid);
        }
    }
    else if (strcasecmp(argv[1], "status") == 0)
    {
        printf("hello from status\n");
    }
    else
    {
        perror("Opção incorreta!\n");
        write(errors,"Opção incorreta!\n",sizeof("Opção incorreta!\n"));
        return 1;
    }

    return 0;
}
