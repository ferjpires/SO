#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>

#include "lib.h"

void exec_status(STATUS status, int parallel_tasks)
{
    //=======================Fork to execute the status==================================
    int id = fork();
    if (id == -1) { handle_error("Fork in exec status\n"); }
    if (id == 0)
    {
        //=======================Opening FIFO==================================
        int sv_to_cl_fifo = open("tmp/sv_to_cl_fifo", O_WRONLY);
        if (sv_to_cl_fifo == -1) { handle_error("Error opening sv_to_cl_fifo in orchestrator\n"); }

        //=======================Passing scheduled processes==================================
        if (write(sv_to_cl_fifo, "\nScheduled:\n", strlen("\nScheduled:\n")) < 0) { handle_error("Write Scheduled failed\n"); }
        for (int i = 0; i < status.queue.tamanho; i++)
        {
            int index = (status.queue.inicio + i) % MAX_ELEMENTS_IN_QUEUE;
            char output[500];
            snprintf(output, sizeof(output), "Process %d, with the command: %s\n", status.queue.values[index].processID, status.queue.values[index].arguments);
            if (write(sv_to_cl_fifo, output, strlen(output)) < 0) { handle_error("Write queued element failed\n"); }
        }

        //=======================Passing executing processes==================================
        if (write(sv_to_cl_fifo, "\nExecuting:\n", strlen("\nExecuting:\n")) < 0) { handle_error("Write Executing failed\n"); }
        for (int i = 0; i < parallel_tasks; i++)
        {
            char output[500];
            snprintf(output, sizeof(output), "Process %d, with the command: %s\n", status.executing[i].processID, status.executing[i].arguments);
            if (write(sv_to_cl_fifo, output, strlen(output)) < 0) { handle_error("Write executing element failed\n"); }
        }

        //=======================Passing completed processes==================================
        if (write(sv_to_cl_fifo, "\nCompleted:\n", strlen("\nCompleted:\n")) < 0) { handle_error("Write Completed failed\n"); }
        for (int i = 0; i < status.finished.tamanho; i++)
        {
            char output[500];
            snprintf(output, sizeof(output), "Process %d, executed in %ld ms, with the command: %s\n", status.finished.values[i].processID, status.finished.values[i].time, status.finished.values[i].arguments);
            if (write(sv_to_cl_fifo, output, strlen(output)) < 0) { handle_error("Write completed element failed\n"); }
        }
        
        //=======================Closing FIFO==================================
        close(sv_to_cl_fifo);
        _exit(1);
    }
}

void execute_program(char *exec_args[], PROGRAM *program, int results, int errors)
{   
    //=======================Setting variables==================================
    struct timeval start, end;
    gettimeofday(&start, NULL);

    //=======================Fork to execute the program==================================
    int id = fork();
    if (id == -1) { handle_error("Fork error in execute_program\n"); }
    if (id == 0)
    {
        dup2(results, 1);
        execvp(exec_args[0], exec_args);
        perror("Failure\n");
        write(errors, "Failure\n", sizeof("Failure\n"));

        _exit(1);
    }
    wait(NULL);

    //=======================Setting information on the program==================================
    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    long milliseconds = (seconds * 1000) + (microseconds / 1000);
    program->time = milliseconds;
    program->running = 2; // done

    //=======================Writing results output==================================
    char output[100];
    snprintf(output, sizeof(output), "Process %d, with time execution of %ld ms\n", program->processID, program->time);
    if (write(results, output, strlen(output)) < 0) { handle_error("Write to results failed\n"); }

    //=======================Opening FIFO==================================
    int main_fifo = open("tmp/main_fifo", O_WRONLY);
    if (main_fifo == -1) { handle_error("Error opening main_fifo in orchestrator\n"); }

    //=======================Writing to save info on status==================================
    int bytes_written = write(main_fifo, program, sizeof(PROGRAM));
    if (bytes_written != sizeof(PROGRAM)) { handle_error("Failed to write the entire program structure to FIFO"); }
    //=======================Closing FIFO==================================
    close(main_fifo);
}

void exec_normal_execute(STATUS *status, PROGRAM *program, int results, int errors)
{
    //=======================Executing array not full===================================
    if (can_execute(status))
    {
        //==================Programs in queue have priority over new======================
        if (waiting_in_queue(status))
        {
            enqueue(&(status->queue), *program); // adds the new
            PROGRAM new_program;
            dequeue(&(status->queue), &new_program); // takes the old

            //=======================Updating executing array===================================
            add_program_to_executing(status, &new_program);

            //=======================Setting variables===================================
            char *exec_args[20];
            parseArguments(*program, exec_args);

            //=======================Fork for parallel execution==============================
            int id = fork();
            if (id == -1) { handle_error("Fork in normal execution, no queue\n"); }
            if (id == 0)
            {
                execute_program(exec_args, &new_program, results, errors);
                _exit(1);
            }
        }
        //=======================Normal execution===================================
        else
        {
            //=======================Updating executing array===================================
            add_program_to_executing(status, program);

            //=======================Setting variables===================================
            char *exec_args[20];
            parseArguments(*program, exec_args);

            //=======================Fork for parallel execution==============================
            int id = fork();
            if (id == -1) { handle_error("Fork in normal execution, no queue\n"); }
            if (id == 0)
            {
                execute_program(exec_args, program, results, errors);
                _exit(1);
            }
        }
    }
    else
    {
        //=======================Executing array full===================================
        if (add_program_to_queue(status, program) != 1) { handle_error("Queue Full\n"); }
        printf("adicionou\n");
    }
}

void exec_pipeline_execute(STATUS *status, PROGRAM *program, int results, int errors)
{
    printf("hello world\n");
}

int main(int argc, char const *argv[])
{
    //=======================Checking conditions==================================
    if (argc != 3) { handle_error("Wrong number of arguments\n"); }
    if (atoi(argv[2]) < 1) { handle_error("Parallel tasks can't be less than 1!\n"); }

    //=======================Setting variables==================================
    char *output_folder = strdup(argv[1]);
    int parallel_tasks = atoi(argv[2]);
    char results_path[100];
    char errors_path[100];
    snprintf(results_path, sizeof(results_path), "%s/%s", output_folder, "/results.txt");
    snprintf(errors_path, sizeof(errors_path), "%s/%s", output_folder, "/errors.txt");

    //=======================Creating files==================================
    mkdir(output_folder, 0777);
    int results = open(results_path, O_RDWR | O_CREAT | O_APPEND, 0666);
    int errors = open(errors_path, O_RDWR | O_CREAT | O_APPEND, 0666);

    //=======================Creating FIFO===================================
    if (mkfifo("tmp/main_fifo", 0666) == -1) { if (errno != EEXIST) { handle_error("Couldn't create main FIFO\n"); } }
    
    //=======================Setting variables==================================
    //PROGRAM program;
    STATUS status;
    create_status(&status, parallel_tasks);
    
    //=======================Main Loop==================================
    while (1)
    {
        //=======================Opening FIFO===================================
        int main_fifo = open("tmp/main_fifo", O_RDONLY | O_NONBLOCK);
        if (main_fifo == -1) { handle_error("Error opening main_fifo in orchestrator\n"); }

        PROGRAM program;
        //=======================Reading data from client=========================
        int bytes_read = read(main_fifo, &program, sizeof(PROGRAM));  
        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available, continue the loop
            } else {
                perror("Read error");
            }
        } 
        else if (bytes_read == sizeof(PROGRAM))
        {
            //=======================Status mode===================================
            if (program.status == 1)
            {
                //STATUS copy = status;
                exec_status(status, parallel_tasks);
                continue;
            }
                
            //==================Checking if program already finished======================
            if (program.running == 2)
            {
                add_program_to_finished(&status, &program);
                continue;
            }

            //=======================Execute mode===================================

            //=======================Normal mode===================================
            if (strcasecmp(program.flag, "-u") == 0)
                exec_normal_execute(&status, &program, results, errors);

            //=======================Pipeline mode===================================
            else
                exec_pipeline_execute(&status, &program, results, errors);
            }

            if (status.queue.tamanho > 0)
            {
                if (can_execute(&status)){
                    PROGRAM queued_program;
                    dequeue(&(status.queue), &queued_program);

                    if (strcasecmp(queued_program.flag, "-u") == 0)
                    {
                        //=======================Updating executing array===================================
                        add_program_to_executing(&status, &queued_program);

                        //=======================Setting variables===================================
                        char *exec_args[20];
                        parseArguments(queued_program, exec_args);

                        //=======================Fork for parallel execution==============================
                        int id = fork();
                        if (id == -1) { handle_error("Fork in normal execution, no queue\n"); }
                        if (id == 0)
                        {
                            execute_program(exec_args, &queued_program, results, errors);
                            _exit(1);
                        }
                    }

                    //=======================Pipeline mode===================================
                    else
                        exec_pipeline_execute(&status, &queued_program, results, errors);
                }
            }

        //=======================Closing FIFO===================================
        close(main_fifo);
    }

    return 0;
}
