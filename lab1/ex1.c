#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int  pid, status;
    pid = fork();
    if (pid!=0)
    { //Pai
        printf("Pai, Pid proprio: %d e Pid do filho: %d\n", getpid(), pid);
        waitpid(-1, &status, 0);
    }else 
    { //Filho
        printf("Filho,  Pid: %d\n", getpid());
        printf("Programa terminado!\n");
        exit(3);
    }
    return 0;
}