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

#include "kernelSim.h"

#define FIFO_KERNEL_IN "kernelFifoInt"

#define FIFO_KERNEL_OUT "kernelFifoOut"

#define TIMESLICE 500000

// cores para output
#define RED  "\x1b[31m"
#define GREEN  "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE  "\x1b[34m"
#define RESET   "\x1b[0m"

int inKernelFIFO, outKernelFIFO;
int interrupcao = 0;

void stopHandler(int signal)
{
    printf(YELLOW"\nController parado!!!\n"RESET);
    interrupcao = 1;
}

void continueHandler(int signal)
{
    // A função intencionalmente não faz nada.
}

void paraPrograma()
{
    char aux = '3';
    write(inKernelFIFO, &aux, 1);

    //pausa de processamento da kernel
    usleep(50000);

    close(outKernelFIFO);

    FILE* fifo_text = fopen(FIFO_KERNEL_OUT, "r");
    if (fifo_text == NULL) 
    {
        perror(RED"Erro na criação do fluxo de arquivo da FIFO!"RESET);
        close(outKernelFIFO);
        exit(0);
    }

    printf("LEGENDA: Estado: 0 - espera ; 1 - em andamento ; 2 terminado ; 3 - bloqueado\nDispositivo: Apenas se estiver bloqueado: 0 - nnao esta bloqueado ; 1 - D1; 2 - D2\nD1 e D2 - Mostra quantas vezes houve interrupções por esses dispositivos\n#------------------------------------------#\n|-PC-|-ESTADO-|-DISPOSITIVO-|-OP-|-D1-|-D2-|\n#------------------------------------------#\n");

    for (int i = 0; i < 5; i++)
    {
        char buffer[30];
        int pc, estado, dispositivo, d1, d2;
        char op;
        if (fgets(buffer, 30, fifo_text) == NULL) 
        {
            fprintf( stderr, RED"Erro ao ler o fluxo da FIFO!\n"RESET, FIFO_KERNEL_OUT);
            break;
        }
        sscanf(buffer, "%d %d %d %c %d %d", &pc, &estado, &dispositivo, &op, &d1, &d2);
        printf("| %d |    %d    |     %d     | %c | %d | %d |\n", pc, estado, dispositivo, op, d1, d2);
    }
    fclose(fifo_text);

    if ((outKernelFIFO = open (FIFO_KERNEL_OUT, O_RDONLY)) < 0)
    {
        fprintf (stderr, RED"Erro ao reabrir a FIFO %s\n"RESET, FIFO_KERNEL_OUT);
        exit(1);
    }

    printf("#------------------------------------------#\n");

    pause();
    interrupcao = 0;
}

int main(void)
{
    //signal(SIGINT, SIG_IGN);
    signal(SIGINT, stopHandler);
    signal(SIGCONT, continueHandler);

    printf(GREEN"Pid do controller: %d\n\n"RESET, getpid());

    // Criação e abertura das FIFOs (comunicação entre kernel e controller)

    if (access(FIFO_KERNEL_OUT, F_OK) == -1)
    {
        if (mkfifo (FIFO_KERNEL_OUT, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, RED"Erro ao criar FIFO %s\n"RESET, FIFO_KERNEL_OUT);
            return -1;
        }
    }
    puts (BLUE"Abrindo FIFO de saída do servidor!"RESET);
    puts (YELLOW"Aguardando iniciação da kernel..."RESET);

    if ((outKernelFIFO = open (FIFO_KERNEL_OUT, O_RDONLY)) < 0)
    {
        fprintf (stderr, RED"Erro ao abrir a FIFO %s\n"RESET, FIFO_KERNEL_OUT);
        return -2;
    }

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

    time_t t;
    srand((unsigned) time(&t));

    for (;;)
    {
        if (interrupcao) paraPrograma();
        char msg_to_kernel;
        int prob1 = rand()%100 + 1;
        int prob2 = rand()%100 + 1;
        

        //IRQ0
        msg_to_kernel = '0';
        write(inKernelFIFO, &msg_to_kernel, 1);
        printf("controller - timeSlice\n");


        //IRQ1
        if (prob1 <= 20) 
        {
            msg_to_kernel = '1';
            write(inKernelFIFO, &msg_to_kernel, 1);
            printf("controller - liberou d1\n");

            //puts("Interrupção do dispositivo 1!");
        }

        //IRQ2
        if (prob2 <= 1) 
        {
            msg_to_kernel = '2';
            write(inKernelFIFO, &msg_to_kernel, 1);
            printf("controller - liberou d2\n");

            //puts("Interrupção do dispotivo 2!");
        };

        printf("controller enviou pela fifo\n");

        //printf("controller mandou pela fifo\n");
        usleep(TIMESLICE);
    }
}
