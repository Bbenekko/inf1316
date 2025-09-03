#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    int segmento;
    char* aux;

    // aloca a memória compartilhada
    segmento = shmget (8752, 100, S_IRUSR | S_IWUSR);

    // associa a memória compartilhada ao processo
    aux = (char *) shmat (segmento, 0, 0); // comparar o retorno com -1

    printf("Conteudo lido de aux:\n %s\n", aux);

    // libera a memória compartilhada do processo
    shmdt (aux);

    // libera a memória compartilhada
    shmctl (segmento, IPC_RMID, 0);
    
    return 0;
}