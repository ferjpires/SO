#ifndef LIB_H
#define LIB_H

#define MAX_ELEMENTS_IN_QUEUE 100
#define MAX_ARGUMENTS_SIZE 300
#define MAX_FINISHED_PROGRAMS 200
#define MAX_FLAG_SIZE 3

typedef struct
{
    int running; // 2 for done, 1 for running, 0 for waiting
    int status;  // 1 for status, 0 for execute
    int expected_time;
    long time;
    char flag[MAX_FLAG_SIZE];
    char arguments[MAX_ARGUMENTS_SIZE];
    int processID;
} PROGRAM;

typedef struct
{
    PROGRAM values[MAX_ELEMENTS_IN_QUEUE];
    int inicio;
    int tamanho;
} QUEUE;

typedef struct
{
    PROGRAM values[MAX_FINISHED_PROGRAMS];
    int tamanho;
} FINISHED;

typedef struct
{
    int current_executing;
    int max_executing;
    PROGRAM *executing;
    QUEUE queue;
    FINISHED finished;
} STATUS;


void handle_error(char *message);

void create_program(PROGRAM *program, char const *argv[], int pid);

void create_status(STATUS *status, int parallel_tasks);

int can_execute(STATUS *status);

int waiting_in_queue(STATUS *status);

int add_program_to_queue(STATUS *status, PROGRAM *program);

int enqueue(QUEUE *queue, PROGRAM value);

int dequeue(QUEUE *queue, PROGRAM *program);

void add_program_to_executing(STATUS *status, PROGRAM *program);

void add_program_to_finished(STATUS *status, PROGRAM *program);

void parseArguments(PROGRAM program, char *exec_args[]);

#endif
