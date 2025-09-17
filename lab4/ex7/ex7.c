#include  "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include <unistd.h>

int main(void)
{
    char*vExecs []= {"aux1", "aux2", "aux3"};
    int vPids[3];
    for (int i = 0; i < 3; i++)
    {
        vPids[i] = fork();
        if ( vPids[i] == 0)
        {
            printf("Estou no filho %d", i);
            execle(vExecs[i], vExecs[i], NULL, (char *)0);
        }
    }

    kill(vPids[0], SIGSTOP);
    kill(vPids[1], SIGSTOP);
    kill(vPids[2], SIGSTOP);

    
    while(1)
    {
        printf(" \n\n\nvai comecar o filho 1:\n\n\n");
        kill(vPids[0], SIGCONT);
        sleep(1);
        kill(vPids[0], SIGSTOP);

        printf(" \n\n\nvai comecar o filho 2:\n\n\n");
        kill(vPids[1], SIGCONT);
        sleep(2);
        kill(vPids[1], SIGSTOP);

        printf(" \n\n\nvai comecar o filho 3:\n\n\n");
        kill(vPids[2], SIGCONT);
        sleep(2);
        kill(vPids[2], SIGSTOP);
    }
    return 0;
}