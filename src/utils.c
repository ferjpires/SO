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
Queue *createQueue()
{
    Queue *queue = (Queue*)malloc(sizeof(Queue));
    queue->front = -1; // Initializing front and rear to -1
    queue->rear = -1;
    return queue;
}

// Function to check if the queue is empty
int isEmpty(Queue *queue)
{
    if (queue->rear == -1)
        return 1;
    else
        return 0;
}

// Function to check if the queue is full
int isFull(Queue *queue)
{
    if (queue->rear == 99)
        return 1;
    else
        return 0;
}

// Function to add an element to the queue
void enqueue(Queue *queue, Program value)
{
    if (isFull(queue))
        printf("Queue is full\n");
    else
    {
        if (isEmpty(queue)) // If queue is empty, set front to 0
            queue->front = 0;
        queue->rear++;
        queue->items[queue->rear] = value;
    }
}

// Function to remove an element from the queue
void dequeue(Queue *queue)
{
    Program item;
    if (isEmpty(queue))
    {
        printf("Queue is empty\n");
    }
    else
    {
        item = queue->items[queue->front];
        queue->front++;
        if (queue->front > queue->rear)
        { // If front becomes greater than rear, reset queue
            queue->front = -1;
            queue->rear = -1;
        }
    }
}

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