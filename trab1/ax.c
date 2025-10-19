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

#define FIFO_AX_OUT "axFifoOut"

#define D1 1
#define D2 0

#define R 0
#define W 1
#define X 2

int PC = 0;
int MAX = 40;
int Dx;
int Op;

int outAxFIFO;

void sysCall(int dispositivo, char operacao)
{
    // pid
    char msg_to_kernel1[16];
    sprintf(msg_to_kernel1, "%d", getpid());
    write(outAxFIFO, msg_to_kernel1, strlen(msg_to_kernel1));

    // PC
    char msg_to_kernel2[3];
    sprintf(msg_to_kernel2, "%d", PC);
    write(outAxFIFO, msg_to_kernel2, strlen(msg_to_kernel2));

    // Dispositivo
    char msg_to_kernel3;
    sprintf(msg_to_kernel3, "%d", dispositivo);
    write(outAxFIFO, msg_to_kernel3, strlen(msg_to_kernel3));

    // Operação
    write(outAxFIFO, operacao, 1);
}

int main(void)
{
    if (access(FIFO_AX_OUT, F_OK) == -1)
    {
        if (mkfifo (FIFO_AX_OUT, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO_AX_OUT);
            return -1;
        }
    }
    if ((outAxFIFO = open (FIFO_AX_OUT, O_RDONLY)) < 0)
    {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO_AX_OUT);
        return -2;
    }

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
        else sysCall(0, '\n');
        sleep(0.1);
        PC++;
        //printf("%d", PC);
    }
    close(outAxFIFO);
}