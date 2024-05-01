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

int execute_program(char *exec_args[], int fd2, int results, int errors, int pipefd)
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
        write(pipefd, &pid, sizeof(int));
        close(pipefd); // Close the write end of the pipe
        write(fd2, &pid, sizeof(int));

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
    if (argc != 3)
    {
        perror("erro nos args\n");
        return 1;
    }
    if (atoi(argv[2]) < 1)
    {
        perror("Número de processos não pode ser inferior a 1!\n");
        return 1;
    }

    char *output_folder = strdup(argv[1]);
    mkdir(output_folder, 0777);

    int parallel_tasks = atoi(argv[2]);

    const char *fifoPath = "tmp/my_fifo";     // Path to the FIFO
    const char *pidFifoPath = "tmp/pid_fifo"; // Path to the process ID FIFO

    char results_path[100];
    snprintf(results_path, sizeof(results_path), "%s/%s", output_folder, "/results.txt");

    char errors_path[100];
    snprintf(errors_path, sizeof(errors_path), "%s/%s", output_folder, "/errors.txt");

    int results = open(results_path, O_RDWR | O_CREAT | O_APPEND, 0666);
    int errors = open(errors_path, O_RDWR | O_CREAT | O_APPEND, 0666);

    // Create the FIFO
    if (mkfifo(fifoPath, 0666) == -1)
    {
        if (errno != EEXIST)
        {
            perror("mkfifo");
            write(errors, "mkfifo\n", sizeof("mkfifo\n"));
            exit(EXIT_FAILURE);
        }
    }
    if (mkfifo(pidFifoPath, 0666) == -1)
    {
        if (errno != EEXIST)
        {
            perror("Error in creating pid Fifo Path\n");
            write(errors, "Error in creating pid Fifo Path\n", sizeof("Error in creating pid Fifo Path\n"));
            exit(EXIT_FAILURE);
        }
    }

    char *exec_args[20];

    Program program;

    int fd = open(fifoPath, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening in orchestrator\n");
        write(errors, "Error opening in orchestrator\n", sizeof("Error opening in orchestrator\n"));
        return 1;
    }
    int fd2 = open(pidFifoPath, O_WRONLY);
    if (fd2 == -1)
    {
        perror("Error opening pid Fifo in orchestrator\n");
        write(errors, "Error opening pid Fifo in orchestrator\n", sizeof("Error opening pid Fifo in orchestrator\n"));
        return 1;
    }

    // Pipe for communicating procID
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    ToUser user_feedback;
    createToUser(&user_feedback,parallel_tasks);

    while (1)
    {
        if (read(fd, &program, sizeof(program)) <= 0)
            continue;             

        if (program.status == 0)
        {
            if (is_there_space(&user_feedback,parallel_tasks) != (-1) && isEmpty(&(user_feedback.in_queue)))
            {
                user_feedback.executing[is_there_space(&user_feedback,parallel_tasks)] = program;

                parseArguments(program, exec_args);

                struct timeval start, end;
                long elapsed_micros;
                gettimeofday(&start, NULL);

                int id = fork();
                if (id == 0)
                {
                    pid_t procID = 0;
                    execute_program(exec_args, fd2, results, errors, pipefd[1]); // Pass the write end of the pipe
                    close(pipefd[1]);                                   // Close the write end of the pipe
                    read(pipefd[0], &procID, sizeof(int));              // Read the procID from the pipe
                    gettimeofday(&end, NULL);
                    elapsed_micros = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

                    char linha[100];
                    int tam = snprintf(linha, sizeof(linha), "Tempo execução: %ld microssegundos\n", elapsed_micros);
                    write(results, linha, tam);

                    tam = snprintf(linha, sizeof(linha), "Process ID: %d\n", procID);
                    write(results, linha, tam);
                }
            }
            else if(is_there_space(&user_feedback,parallel_tasks) != (-1) && !isEmpty(&(user_feedback.in_queue)))
            {
                enqueue(&(user_feedback.in_queue),program);

                Program prog_from_queue = user_feedback.in_queue.values[user_feedback.in_queue.inicio];
                user_feedback.executing[is_there_space(&user_feedback,parallel_tasks)] = prog_from_queue;
                
                parseArguments(prog_from_queue, exec_args);

                struct timeval start, end;
                long elapsed_micros;
                gettimeofday(&start, NULL);

                int id = fork();
                if (id == 0)
                {
                    pid_t procID = 0;
                    execute_program(exec_args, fd2, results, errors, pipefd[1]); // Pass the write end of the pipe
                    close(pipefd[1]);                                   // Close the write end of the pipe
                    read(pipefd[0], &procID, sizeof(int));              // Read the procID from the pipe
                    gettimeofday(&end, NULL);
                    elapsed_micros = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

                    char linha[100];
                    int tam = snprintf(linha, sizeof(linha), "Tempo execução: %ld microssegundos\n", elapsed_micros);
                    write(results, linha, tam);

                    tam = snprintf(linha, sizeof(linha), "Process ID: %d\n", procID);
                    write(results, linha, tam);
                }

                dequeue(&(user_feedback.in_queue));
            }
            else
            {
                enqueue(&(user_feedback.in_queue),program);
            }
        }
        else
        {
            printf("O server está no status\n");
        }
    }

    return 0;
}