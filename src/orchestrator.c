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
#include <sys/wait.h> // For wait function
#include "lib.h"

int tryone(char *exec_args[], int fd2, int results,int errors)
{
    int status;
    int id = fork();
    if (id == -1)
    {
        perror("Fork error in orchestrator\n");
        write(errors, "Fork error in orchestrator\n", sizeof("Fork error in orchestrator\n"));
        return 1;
    }

    if (id == 0) // Child Process
    {
        pid_t pid = getpid();
        write(fd2, &pid, sizeof(int));
        char linha[100];
        int tam = snprintf(linha, sizeof(linha), "Process ID: %d\n", pid);
        write(results, linha, tam);
        dup2(results, 1);

        execvp(exec_args[0], exec_args);
        perror("Failure\n");
        write(errors, "Failure\n", sizeof("Failure\n"));
        
        _exit(1);
    }
    pid_t pid = wait(&status);
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }
    else
    {
        char buffer[50];
        int len = sprintf(buffer, "son: %d died\n", pid);
        write(STDOUT_FILENO, buffer, len);
        return -1;
    }
}

int main(int argc, char const *argv[])
{
    const char *fifoPath = "tmp/my_fifo";     // Path to the FIFO
    const char *pidFifoPath = "tmp/pid_fifo"; // Path to the process ID FIFO

    int results = open("src/results.txt", O_RDWR | O_CREAT | O_APPEND, 0666);
    int errors = open("src/errors.txt", O_RDWR | O_CREAT | O_APPEND, 0666);

    // Create the FIFO
    if (mkfifo(fifoPath, 0666) == -1)
    {
        if (errno != EEXIST)
        {
            perror("mkfifo");
            write(errors,"mkfifo\n",sizeof("mkfifo\n"));
            exit(EXIT_FAILURE);
        }
    }
    if (mkfifo(pidFifoPath, 0666) == -1)
    {
        if (errno != EEXIST)
        {
            perror("Error in creating pid Fifo Path\n");
            write(errors,"Error in creating pid Fifo Path\n",sizeof("Error in creating pid Fifo Path\n"));
            exit(EXIT_FAILURE);
        }
    }

    char *exec_args[20];

    Program program;

    int fd = open(fifoPath, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening in orchestrator\n");
        write(errors,"Error opening in orchestrator\n",sizeof("Error opening in orchestrator\n"));
        return 1;
    }
    int fd2 = open(pidFifoPath, O_WRONLY);
    if (fd2 == -1)
    {
        perror("Error opening pid Fifo in orchestrator\n");
        write(errors,"Error opening pid Fifo in orchestrator\n",sizeof("Error opening pid Fifo in orchestrator\n"));
        return 1;
    }

    while (1)
    {
        if (read(fd, &program, sizeof(program)) <= 0)
            continue; // tive de por assim porque assim não sai do while nem continua a escrever o mesmo

        parseArguments(program, exec_args);

        struct timeval start, end;
        long elapsed_micros;
        gettimeofday(&start, NULL);

        /* int id = fork();
        if (id == -1)
        {
            perror("Fork error in orchestrator\n");
            return 1;
        }

        if (id == 0) // Child Process
        {
            int pid = getpid();
            write(fd2, &pid, sizeof(int));
            dup2(results, 1);

            execvp(exec_args[0], exec_args);
            perror("Failure\n");
            _exit(1);
        }
        pid_t pid = wait(&status);
        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        else
        {
            printf("son: %d died\n", pid);
            return -1;
        } */
        int id = fork();
        if (id == 0)
        {
            tryone(exec_args, fd2, results,errors);
            gettimeofday(&end, NULL);
            elapsed_micros = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

            char linha[100];
            int tam = snprintf(linha, sizeof(linha), "Tempo execução: %ld microssegundos\n", elapsed_micros);

            write(results, linha, tam);
        }

        // sleep(3);
    }

    return 0;
}