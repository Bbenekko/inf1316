#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define FIFO_KERNEL_IN "kernelFifoInt"
#define FIFO_KERNEL_OUT "kernelFifoOut"

#define TIMESLICE 500

// cores para output
#define RED  "\x1b[31m"
#define GREEN  "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE  "\x1b[34m"
#define RESET   "\x1b[0m"

int inKernelFIFO, outKernelFIFO;

void quitHandler(int sinal)
{
    char ch;
    write(inKernelFIFO, "stop", 5);
    while (read (outKernelFIFO, &ch, sizeof(ch)) == 1 && ch != '\n') putchar(ch);
    kill(SIGSTOP);
}

int main(void)
{
    void (*p)(int); 

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
    if ((inKernelFIFO = open (FIFO_KERNEL_IN, O_RDONLY)) < 0)
    {
        fprintf (stderr, RED"Erro ao abrir a FIFO %s\n"RESET, FIFO_KERNEL_IN);
        return -2;
    }

    if (access(FIFO_KERNEL_OUT, F_OK) == -1)
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
    }

    time_t t;
    srand((unsigned) time(&t));

    p = signal(SIGQUIT, quitHandler);

    for (;;)
    {
        char* msg_to_kernel = "   ";
        int index = 0;
        int prob1 = rand()%100 + 1;
        int prob2 = (rand()+1)%100 + 1;
        usleep(TIMESLICE);

        //IRQ0
        msg_to_kernel[index] = '0';
        index++;

        //IRQ1
        if (prob1 <= 10) 
        {
            msg_to_kernel[index] = '1';
            index++;
            puts("Interrupção do dispositivo 1!");
        }

        //IRQ2
        if (prob2 <= 5) 
        {
            msg_to_kernel[index] = '2';
            index++;
            puts("Interrupção do dispotivo 2!");
        };

        write(inKernelFIFO, msg_to_kernel, strlen(msg_to_kernel));
    }
}