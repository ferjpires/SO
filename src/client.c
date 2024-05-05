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

void handle_status()
{
    //=======================Creating FIFO===================================
    if (mkfifo("tmp/sv_to_cl_fifo", 0666) == -1) { if (errno != EEXIST) { handle_error("Couldn't create sv_to_cl FIFO\n"); } }
    
    //=======================Opening FIFO===================================
    int sv_to_cl_fifo = open("tmp/sv_to_cl_fifo", O_RDONLY);
    if (sv_to_cl_fifo == -1) { handle_error("Error opening sv_to_cl_fifo in client\n"); }

    //=======================Setting variables==================================
    char buffer[500];
    ssize_t bytes_read;

    //=======================Writing to user==================================
    while ((bytes_read = read(sv_to_cl_fifo, buffer, sizeof(buffer))) > 0)
        write(STDOUT_FILENO, buffer, bytes_read);

    //=======================Closing FIFO===================================
    close(sv_to_cl_fifo);
    unlink("tmp/sv_to_cl_fifo");
}

void status_mode(int argc)
{
    //=======================Opening FIFO===================================
    int main_fifo = open("tmp/main_fifo", O_WRONLY);
    if (main_fifo == -1) { handle_error("Error opening main_fifo in client\n"); }

    //=======================Checking conditions==================================
    if (argc != 2) { handle_error("Invalid number of arguments in status\n"); }

    //=======================Setting variables==================================
    PROGRAM program;
    program.status = 1;
    program.processID = getpid();
    program.running = 0;
    strcpy(program.flag, "");
    strcpy(program.arguments,"");

    //=======================Writing to server==================================
    write(main_fifo, &program, sizeof(PROGRAM));

    //=======================Closing FIFO========================================
    close(main_fifo);

    //=======================Handling server feedback==================================
    handle_status();
}

void execute_mode(int argc, char const *argv[])
{
    //=======================Opening FIFO===================================
    int main_fifo = open("tmp/main_fifo", O_WRONLY);
    if (main_fifo == -1) { handle_error("Error opening main_fifo in client\n"); }

    //=======================Checking conditions==================================
    if (argc != 5) { handle_error("Invalid number of arguments in execute\n"); }
    if (atoi(argv[2]) < 1) { handle_error("Time (ms) can't be less than 1\n"); }

    //=======================Setting variables==================================
    PROGRAM program;
    int pid = getpid();
    create_program(&program, argv, pid);

    //=======================Writing to user==================================
    char output[100];
    snprintf(output, sizeof(output), "Process %d has just started\n", program.processID);
    write(STDOUT_FILENO, output, strlen(output));

    //=======================Writing to server==================================
    write(main_fifo, &program, sizeof(PROGRAM));

    //=======================Closing FIFO===================================
    close(main_fifo);

}

int main(int argc, char const *argv[])
{
    //=======================Checking conditions==================================
    if (argc < 2) { handle_error("Invalid number of arguments\n"); }

    //=======================Execute mode===================================
    if (strcasecmp(argv[1], "execute") == 0)
        execute_mode(argc, argv);

    //=======================Status mode===================================
    else if (strcasecmp(argv[1], "status") == 0)
        status_mode(argc);

    return 0;
}
