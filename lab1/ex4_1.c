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
        if (waitpid(pid, &status, 0) == pid)
        {
            printf("Programa terminado com retorno em 0!\n");
        }
    }else 
    { //Filho
        printf("Filho,  Pid: %d\n", getpid());
        execle("/home/neco/Documents/sistemas_operacionais/lab1/hello", "/home/neco/Documents/sistemas_operacionais/lab1/hello", NULL, (char *)0);
        printf("Programa terminado!\n");
        exit(3);
    }
    return 0;
}