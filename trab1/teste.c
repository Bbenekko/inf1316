#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

struct teste
{
    int valor;
    int valor2;
} typedef teste;

int main (int argc, char *argv[])
{
    int segmento;
    FILE* arq;
    teste* aux;

    // aloca a memória compartilhada
    segmento = shmget (8752, sizeof(teste)*4, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    // associa a memória compartilhada ao processo
    aux = (teste *) shmat (segmento, 0, 0); // comparar o retorno com -1
    
    aux[0].valor = 10;
    aux[1].valor2 = 20;

    printf("Conteudo inserido em aux:\n %d  %d\n", aux[0].valor, aux[1].valor2);

    shmdt(aux);

    // libera a memória compartilhada do processo
    //shmdt (aux);

    // libera a memória compartilhada
    //shmctl (segmento, IPC_RMID, 0);
    
    return 0;
}