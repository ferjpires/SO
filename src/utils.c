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

void handle_error(char *message)
{
    char error[100];
    snprintf(error, sizeof(error), "%s%s", "Error: ", message);
    perror(error);
    exit(EXIT_FAILURE);
}

void create_program(PROGRAM *program, char const *argv[], int pid)
{
    program->expected_time = atoi(argv[2]);
    program->processID = pid;
    program->status = 0;
    program->running = 0;
    strcpy(program->flag, argv[3]);
    strcpy(program->arguments, argv[4]);
}

void create_status(STATUS *status, int parallel_tasks)
{
    struct timeval start;
    gettimeofday(&start, NULL);
    long milliseconds = (start.tv_sec * 1000) + (start.tv_usec / 1000);
    status->start_time = milliseconds;
    status->executing = malloc(parallel_tasks * sizeof(PROGRAM));
    status->current_executing = 0;
    status->max_executing = parallel_tasks;
    status->queue.inicio = 0;
    status->queue.tamanho = 0;
    status->finished.tamanho = 0;
}

int can_execute(STATUS *status)
{
    if (status->current_executing < status->max_executing)
        return 1;
    return 0;
}

int waiting_in_queue(STATUS *status)
{
    if (status->queue.tamanho > 0)
        return 1;
    return 0;
}

// returns 1 if success, 0 if not
int add_program_to_queue(STATUS *status, PROGRAM *program)
{
    return(enqueue(&(status->queue), *program));
}

PROGRAM* find_fastest_program_queue(QUEUE *queue) {
    if (queue->tamanho == 0) {
        return NULL; // Queue is empty
    }

    PROGRAM *fastest = &(queue->values[queue->inicio]);
    int i;
    for (i = 1; i < queue->tamanho; i++) {
        PROGRAM *current_program = &(queue->values[(queue->inicio + i) % MAX_ELEMENTS_IN_QUEUE]);
        if (current_program->expected_time < fastest->expected_time) {
            fastest = current_program;
        }
    }

    return fastest;
}

// Function to add an element to the queue
// 1 if success, 0 if not
int enqueue(QUEUE *queue, PROGRAM value)
{
    if (queue->tamanho == MAX_ELEMENTS_IN_QUEUE)
        return 0;

    queue->values[(queue->inicio + queue->tamanho++) % MAX_ELEMENTS_IN_QUEUE] = value;
    return 1;
}

// Function to remove an element from the queue
// 1 if success, 0 if not
int dequeue(QUEUE *queue, PROGRAM *program)
{
    if (queue->tamanho == 0)
        return 0;
    *program = queue->values[queue->inicio];
    queue->inicio = (queue->inicio + 1) % MAX_ELEMENTS_IN_QUEUE;
    queue->tamanho--;
    return 1;
}

int dequeue_fastest_program(QUEUE *queue, PROGRAM *program) {
    if (queue->tamanho == 0) {
        return 0;
    }

    PROGRAM *fastest = find_fastest_program_queue(queue);

    *program = *fastest;

    int i, found_index = -1;
    for (i = 0; i < queue->tamanho; i++) {
        if (&(queue->values[(queue->inicio + i) % MAX_ELEMENTS_IN_QUEUE]) == fastest) {
            found_index = i;
            break;
        }
    }

    for (i = found_index; i < queue->tamanho - 1; i++) {
        queue->values[(queue->inicio + i) % MAX_ELEMENTS_IN_QUEUE] = queue->values[(queue->inicio + i + 1) % MAX_ELEMENTS_IN_QUEUE];
    }

    queue->tamanho--;

    return 1; 
}

void add_program_to_executing(STATUS *status, PROGRAM *program)
{
    for (int i = 0; i < status->max_executing; i++)
    {
        if (status->executing[i].running != 1)
        {
            program->running = 1;
            status->executing[i] = *program;
            status->current_executing++;
            break;
        }
    }
}

void add_program_to_finished(STATUS *status, PROGRAM *program)
{
    PROGRAM new_program = *program;
    status->finished.values[status->finished.tamanho++] = new_program;
    for (int i = 0; i < status->max_executing; i++)
    {
        if (status->executing[i].processID == program->processID)
        {
            status->executing[i].processID = 0;
            strcpy(status->executing[i].arguments, "");
            status->executing[i].running = 2;
            break;
        }
    }
    status->current_executing--;
}

void parseArguments(PROGRAM program, char *exec_args[])
{
    char *args;
    args = strdup(program.arguments);
    int i = 0;
    char *string;
    string = strsep(&args, " ");
    while (string != NULL)
    {
        exec_args[i] = string;
        string = strsep(&args, " ");
        i++;
    }
    exec_args[i] = NULL;
}
