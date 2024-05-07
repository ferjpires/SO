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

        //=======================Passing completed processes==================================
        if (write(sv_to_cl_fifo, "\nTime so far:\n", strlen("\nTime so far:\n")) < 0) { handle_error("Write Completed failed\n"); }
        struct timeval end;
        gettimeofday(&end, NULL);
        long milliseconds = (end.tv_sec * 1000) + (end.tv_usec / 1000);
        long timePassed = milliseconds - status.start_time;
        char output[500];
        snprintf(output, sizeof(output), "%ld ms\n", timePassed);
        if (write(sv_to_cl_fifo, output, strlen(output)) < 0) { handle_error("Write completed element failed\n"); }

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
        //printf("adicionou\n");
    }
}

int parse_commands(char *cmd, char *cmds[25])
{
    int num_cmds = 0;
    char *token = strtok(cmd, "|");
    while (token != NULL && num_cmds < 25)
    {
        cmds[num_cmds++] = token;
        token = strtok(NULL, "|");
    }
    return num_cmds;
}

void execute_commands(PROGRAM *program,int num_cmds, char *cmds[25], int results)
{
    //=======================Setting variables===================================
    int i;
    int num_pipes = num_cmds - 1;
    int pipefds[2 * num_pipes];
    struct timeval start, end;
    gettimeofday(&start, NULL);

    //=======================Creating the pipes===================================
    for (i = 0; i < num_pipes; i++)
        if (pipe(pipefds + i * 2) == -1) { handle_error("Failed to create pipes\n"); }
    //=======================Main loop for number of commands===================================
    for (i = 0; i < num_cmds; i++)
    {
        //=======================Forking to execute a command===================================
        pid_t pid = fork();
        if (pid == -1) { handle_error("Failed to fork process in pipeline\n"); }

        if (pid == 0)
        {
            //==========Not the first command and there's a previous pipe=================
            if (i > 0)
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);

            //==========Not the last command and there's a next pipe=================
            if (i < num_cmds - 1)
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            else
                // Last command outputs to the specified file
                dup2(results, STDOUT_FILENO);

            //=======================Closing the pipes===================================
            for (int j = 0; j < 2 * num_pipes; j++)
                close(pipefds[j]);

            //=======================Parsing arguments===================================
            char *sub_cmds[20];
            int k = 0;
            char *token = strtok(cmds[i], " ");
            while (token != NULL && k < 20) {
                sub_cmds[k++] = token;
                token = strtok(NULL, " ");
            }
            sub_cmds[k] = NULL;

            //=======================Executing the command===================================
            execvp(sub_cmds[0], sub_cmds);
            perror("execvp");
            _exit(1);
        }
    }    
        //=======================Parent closing the pipes===================================
        for (i = 0; i < 2 * num_pipes; i++)
            close(pipefds[i]);

        //====================Parent waits for all child processes=============================
        for (i = 0; i < num_cmds; i++)
            wait(NULL);

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

void exec_pipeline_execute(STATUS *status, PROGRAM *program, int results, int errors)
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
            char *cmds[25];
            int num_cmds = parse_commands(program->arguments, cmds);

            //=======================Fork for parallel execution==============================
            int id = fork();
            if (id == -1) { handle_error("Fork in normal execution, no queue\n"); }
            if (id == 0)
            {
            execute_commands(program,num_cmds, cmds, results);
            exit(1);
            }
        }
        //=======================Normal execution===================================
        else
        {
            //=======================Updating executing array===================================
            add_program_to_executing(status, program);

            //=======================Setting variables===================================
            char *cmds[25];
            int num_cmds = parse_commands(program->arguments, cmds);


            //=======================Fork for parallel execution==============================
            int id = fork();
            if (id == -1) { handle_error("Fork in normal execution, no queue\n"); }
            if (id == 0)
            {
            execute_commands(program , num_cmds, cmds, results);
            exit(1);
            }
        }
    }
    else
    {
        //=======================Executing array full===================================
        if (add_program_to_queue(status, program) != 1) { handle_error("Queue Full\n"); }
    }
}

int main(int argc, char const *argv[])
{
    //=======================Checking conditions==================================
    if (argc != 4) { handle_error("Wrong number of arguments\n"); }
    if (atoi(argv[2]) < 1) { handle_error("Parallel tasks can't be less than 1!\n"); }
    if (atoi(argv[3]) > 1 || atoi(argv[3]) < 0) { handle_error("There are only politic 0 and 1\n"); }

    //=======================Setting variables==================================
    char *output_folder = strdup(argv[1]);
    int parallel_tasks = atoi(argv[2]);
    int politics = atoi(argv[3]); 
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
        if (status.queue.tamanho > 0)
        {
            if (can_execute(&status))
            {
                PROGRAM queued_program;
                if(politics == 0)
                dequeue(&(status.queue), &queued_program);
                else dequeue_fastest_program(&(status.queue), &queued_program);

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
                else{
                    //=======================Updating executing array===================================
                     add_program_to_executing(&status, &queued_program);

                    //=======================Setting variables===================================
                    char *cmds[25];
                    int num_cmds = parse_commands(queued_program.arguments, cmds);


                    //=======================Fork for parallel execution==============================
                    int id = fork();
                    if (id == -1) { handle_error("Fork in normal execution, no queue\n"); }
                    if (id == 0)
                    {
                    execute_commands(&queued_program , num_cmds, cmds, results);
                    exit(1);
                    }
                }  
            }
        }

        //=======================Opening FIFO===================================
        int main_fifo = open("tmp/main_fifo", O_RDONLY ); // | O_NONBLOCK
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

            // if (status.queue.tamanho > 0)
            // {
            //     if (can_execute(&status)){
            //         PROGRAM queued_program;
            //         dequeue(&(status.queue), &queued_program);

            //         if (strcasecmp(queued_program.flag, "-u") == 0)
            //         {
            //             //=======================Updating executing array===================================
            //             add_program_to_executing(&status, &queued_program);

            //             //=======================Setting variables===================================
            //             char *exec_args[20];
            //             parseArguments(queued_program, exec_args);

            //             //=======================Fork for parallel execution==============================
            //             int id = fork();
            //             if (id == -1) { handle_error("Fork in normal execution, no queue\n"); }
            //             if (id == 0)
            //             {
            //                 execute_program(exec_args, &queued_program, results, errors);
            //                 _exit(1);
            //             }
            //         }

            //         //=======================Pipeline mode===================================
            //         else
            //             exec_pipeline_execute(&status, &queued_program, results, errors);
            //     }
            // }

        //=======================Closing FIFO===================================
        close(main_fifo);
    }

    return 0;
}
