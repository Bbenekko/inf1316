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
    FILE* arq;
    char* aux;

    // aloca a memória compartilhada
    segmento = shmget (8752, 100, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    // associa a memória compartilhada ao processo
    aux = (char *) shmat (segmento, 0, 0); // comparar o retorno com -1
    
    arq = fopen("arq_ex2.txt", "r");
    fgets(aux, 100, arq);
    fclose(arq);

    printf("Conteudo inserido em aux:\n %s\n", aux);

    shmdt(aux);

    // libera a memória compartilhada do processo
    //shmdt (aux);

    // libera a memória compartilhada
    //shmctl (segmento, IPC_RMID, 0);
    
    return 0;
}