#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define FIFO_AX_OUT "axFifoOut"

#define WOPENMODE (O_WRONLY | O_NONBLOCK)
#define MAX 5

int main(void)
{
    signal(SIGINT, SIG_IGN);
    pause();

    time_t t;
    srand((unsigned) time(&t));

    int PC = 0;

    if (access(FIFO_AX_OUT, F_OK) == -1) {
        if (mkfifo(FIFO_AX_OUT, S_IRUSR | S_IWUSR) != 0) {
            perror("mkfifo axFifoOut"); return 1;
        }
    }
    int outAxFIFO = open(FIFO_AX_OUT, O_WRONLY);
    if (outAxFIFO < 0) { perror("open axFifoOut"); return 1; }

    while (PC < MAX) {
        int dispositivo = 0;
        int estado = 1;
        char operacao = 'x'; 
        usleep(100000);  // 0.1 s
        estado = 1;
        PC++;
        int d = rand()%100 + 1;

        if (d < 15) {
            dispositivo = ((d % 2) ? 1 : 0) + 1;
            operacao = "rw"[rand()%2];
        } 

        if (pIda->valorPC >= MAX)
        {
            estado = 2;
        }

        char buf[200];


        sprintf(buf, "%d %d %d %d %c\n", getpid(), PC, dispositivo, estado, operacao);
        write(outAxFIFO, buf, strlen(buf) + 1);
        printf("Buffer de AX: %s\n", buf);
      
        usleep(300000);
    }
    close(outAxFIFO);
    return 0;
}

