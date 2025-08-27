#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("hello,  Pid: %d, Pid do pai: %d\n", getpid(), getppid());
    printf("Alo mundo!\n");
    return 0;
}