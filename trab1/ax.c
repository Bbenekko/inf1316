#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include "kernelSim.h"


#define D1 1
#define D2 0

#define R 0
#define W 1
#define X 2

int PC = 0;
int MAX = 40;
int Dx;
int Op;

int main(void)
{

     //area da memoria compartilhada para compartilhar os status dos processos
    __key_t chave = 8751;
    int mv = shmget (chave, sizeof (Info) * 5, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWGRP);
    Info* vInfoComp = (Info *) shmat (mv, 0, 0);

    while (PC < MAX) {
        sleep(0.1);
        int d = rand()%100 +1;
        if (d  < 15) { // generate a random sysCall
            if (d % 2) Dx = D1;
            else Dx= D2;
            if (d % 3 == 1) Op = R;
            else if (d % 3 == 1) Op = W;
            else Op = X;
            sysCall (Dx, Op);
        }
        sleep(0.1);
        PC++;
        printf("%d", PC);
    }
}