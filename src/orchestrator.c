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
#include <sys/wait.h>  // For wait function
#include "lib.h"

int main(int argc, char const *argv[])
{
    const char *fifoPath = "tmp/my_fifo"; // Path to the FIFO

     // Create the FIFO
    if (mkfifo(fifoPath, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    char *exec_args[20]; 

    Program programa;

    int fd = open(fifoPath, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    int results = open("src/results.txt", O_RDWR | O_CREAT | O_APPEND);

    while (read(fd, &programa, sizeof(programa)) > 0)
    {
        parseArguments(programa, exec_args);

        int id = fork();

        if (id == 0) // Child Process
        {
            dup2(results, 1);
            execvp(exec_args[0], exec_args); 
            _exit(1);
        }
        else // Parent Process
        {
            wait(NULL);
        }
        sleep(3);
    }

    return 0;
}