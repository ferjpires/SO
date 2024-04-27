#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <lib.h>

#define FIFO_PATH "../tmp/TUBO"

int main(int argc, char const *argv[])
{
   
   char exec_args[20][300];
   Program programa;

    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    int results = open("results.txt", O_RDWR | O_CREAT | O_APPEND);

    while (read(fd,&programa,sizeof(programa)) > 0)
    {
        parseArguments(programa,exec_args);

        int id = fork();

        if (id == 0) //Child Process
        {
            dup2(results,1);
            execvp(1,exec_args);
            _exit(1);
        }
        else //Parent Process
        {
            wait(NULL);
        }
        sleep(3);
    }

    return 0;
}
