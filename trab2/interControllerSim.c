#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "trabalho.h"

#define FIFO_KERNEL_IN "kernelFifoInt"
//#define FIFO_KERNEL_OUT "kernelFifoOut"

#define TIMESLICE 500

// cores para output
#define RED  "\x1b[31m"
#define GREEN  "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE  "\x1b[34m"
#define RESET   "\x1b[0m"

int stopFlag = 0;

int inKernelFIFO, outKernelFIFO;

void iniciaVetor(Info vInfos[])
{
    for(int i = 0; i < 5; i++)
    {
        vInfos[i].dispositivo = 0;
        vInfos[i].estado = 0;
        vInfos[i].estaTerminado = 0;
        vInfos[i].qtdVezesParado = 0;
        vInfos[i].valorPC = 0;
        vInfos[i].operacao = '\0';
        vInfos[i].qtdVzsD1 = 0;
        vInfos[i].qtdVzsD2 = 0;
    }
}

void stopHandler(int sinal)
{
    stopFlag = 1;
}

int main(void)
{
    void (*p)(int); 

    printf(GREEN"Pid do controller: %d\n\n"RESET, getpid());

    // Criação e abertura das FIFOs (comunicação entre kernel e controller)
    if (access(FIFO_KERNEL_IN, F_OK) == -1)
    {
        if (mkfifo (FIFO_KERNEL_IN, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, RED"Erro ao criar FIFO %s\n"RESET, FIFO_KERNEL_IN);
            return -1;
        }
    }
    puts (BLUE"Abrindo FIFO de entrada do servidor!"RESET);
    puts (YELLOW"Aguardando iniciação da kernel..."RESET);
    if ((inKernelFIFO = open (FIFO_KERNEL_IN, O_WRONLY)) < 0)
    {
        fprintf (stderr, RED"Erro ao abrir a FIFO %s\n"RESET, FIFO_KERNEL_IN);
        return -2;
    }

    /* if (access(FIFO_KERNEL_OUT, F_OK) == -1)
    {
        if (mkfifo (FIFO_KERNEL_OUT, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr,RED "Erro ao criar FIFO %s\n"RESET, FIFO_KERNEL_OUT);
            return -1;
        }
    }
    puts (BLUE"Abrindo FIFO de saida do servidor!"RESET);
    if ((outKernelFIFO = open (FIFO_KERNEL_OUT, O_WRONLY)) < 0)
    {   
        fprintf (stderr, RED"Erro ao abrir a FIFO %s\n"RESET, FIFO_KERNEL_OUT);
        return -2;
    } */

    time_t t;
    srand((unsigned) time(&t));

    p = signal(SIGQUIT, stopHandler);

    for (;;)
    {
        if(stopFlag)
        {
            write(inKernelFIFO, "stop", 5);

            int segmento;
            __key_t chave = 8751;

            segmento = shmget(chave, sizeof(Info) * 5, S_IRUSR | S_IWUSR);
            Info* vInfo = (Info*)shmat(segmento, 0, 0);
            iniciaVetor(vInfo);
    
    
            printf("#------------------------------------------------------------------#\n|-PC-|-ESTADO-|-BLOQUEADO-|-DISPOSITIVO-|-OP-|-D1-|-D2-|-TERMINADO-|\n#------------------------------------------------------------------#\n");

            for (int i = 0; i < 5; i++)
            {
                printf("| %d |   %d   |   %d   |  %d  | %d | %d | %d |   %d   |\n", vInfo[i].valorPC, vInfo[i].estado, vInfo[i].estado, vInfo[i].dispositivo, vInfo[i].operacao, vInfo[i].qtdVzsD1, vInfo[i].qtdVzsD2, vInfo[i].estaTerminado);
            }

            //printf("| %d | %s |   %d   |  %s  | %s |    %d    | %d | %d | %d |", pc, estado, bloqueado, dispositivo, op, executando, d1, d2, terminado);
            printf("#------------------------------------------------------------------#\n");

            shmdt(vInfo);

            pause();

            stopFlag = 0;
        }

        char msg_to_kernel[4];
        int index = 0;
        int prob1 = rand()%100 + 1;
        int prob2 = rand()%100 + 1;
        

        //IRQ0
        msg_to_kernel[index] = '0';
        index++;

        //IRQ1
        if (prob1 <= 20) 
        {
            msg_to_kernel[index] = '1';
            index++;
            //puts("Interrupção do dispositivo 1!");
        }

        //IRQ2
        if (prob2 <= 1) 
        {
            msg_to_kernel[index] = '2';
            index++;
            //puts("Interrupção do dispotivo 2!");
        };

        msg_to_kernel[index] = '\0';
        index++;

        //printf("controller vai mandar pela fifo\n");
        write(inKernelFIFO, msg_to_kernel, index);
        //printf("controller mandou pela fifo\n");
        usleep(TIMESLICE * 1000);
    }
}
