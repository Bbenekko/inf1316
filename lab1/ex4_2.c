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
        if (waitpid(pid, &status, 0) == pid)
        {
            printf("Programa terminado com retorno em 0!\n");
        }
    }else 
    { //Filho
        system("echo Alo mundo!");
        printf("Programa terminado!\n");
        exit(3);
    }
    return 0;
}