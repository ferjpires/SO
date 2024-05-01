#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "lib.h"

void create_program(Program *program, char const *argv[])
{
    program->time = atoi(argv[2]);
    strcpy(program->flag, argv[3]);
    strcpy(program->arguments, argv[4]);
}

void parseArguments(Program program, char *exec_args[])
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

// Function to create a queue
void initQueue (Queue *q) {
q->inicio = 0; 
q->tamanho = 0;
}

// Function to check if the queue is empty
int isEmpty(Queue *queue)
{
    return(queue->tamanho == 0);
}

// Function to check if the queue is full
int isFull(Queue *queue)
{
    return (queue->tamanho == MAX);
}

// Function to add an element to the queue
void enqueue(Queue *queue, Program value)
{
    int r =0;
    if (queue->tamanho == MAX) r = 1;
    else
    queue->values[(queue->inicio + queue->tamanho++)%MAX] = value;
    return r ;
}

// Function to remove an element from the queue
int dequeue ( Queue *queue) {
if(queue->tamanho == 0) printf("vazia\n");
else{
queue->inicio = (queue->inicio + 1)%MAX;
queue->tamanho--;
}
}

/*
// Function to display the elements of the queue
void display(Queue* queue) {
    int i;
    if (isEmpty(queue))
        printf("Queue is empty\n");
    else {
        printf("Queue elements are:\n");
        for (i = queue->front; i <= queue->rear; i++)
        {
            printf("%d\n", queue->items[i].processID);
            printf("%s\n", queue->items[i].arguments);
        }
}
}
*/