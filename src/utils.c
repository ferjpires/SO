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
    if (queue->tamanho == MAX) printf("Cheia\n");
    else
    queue->values[(queue->inicio + queue->tamanho++)%MAX] = value;
}

// Function to remove an element from the queue
void dequeue (Queue *queue) {
if(queue->tamanho == 0) printf("vazia\n");
else{
queue->inicio = (queue->inicio + 1)%MAX;
queue->tamanho--;
}
}

void createToUser(ToUser *touser, int parallel){
initQueue(&(touser->in_queue)); // pass the address of in_queue
touser->executing = malloc(parallel * sizeof(Program)); 
for (int i = 0; i < parallel; i++)
{
    Program prog;
    prog.running = 0; 
    touser->executing[i] = prog;
}
touser->finished.tamanho = 0;
}

int is_there_space(ToUser *touser,int parallel){
    for (int i = 0; i < parallel; i++)
    {
        if(touser->executing[i].running == 0) return i;
    }
    return -1;
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