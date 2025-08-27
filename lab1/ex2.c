#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int  pid, status;
    int var = 1;
    printf("Variavel: %d\n", var);
    pid = fork();
    if (pid!=0)
    { //Pai
        printf("Pai, Pid proprio: %d e Pid do filho: %d\n", getpid(), pid);
        waitpid(-1, &status, 0);
        printf("Pai: Variavel: %d\n", var);

    }else 
    { //Filho
        printf("Filho,  Pid: %d\n", getpid());

        var = 5;
        printf("Filho: Variavel: %d\n", var);

        printf("Programa terminado!\n");
        exit(3);
    }
    return 0;
}