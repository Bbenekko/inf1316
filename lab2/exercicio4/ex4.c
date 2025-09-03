#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    int segmento1, segmento2, *p1, *p2, id, id2, pid, status;

    // aloca a memória compartilhada
    segmento1 = shmget (IPC_PRIVATE, sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    // associa a memória compartilhada ao processo
    p1 = (int *) shmat (segmento1, 0, 0); // comparar o retorno com -1
    *p1 = 0;

    segmento2 = shmget (IPC_PRIVATE, sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    p2 = (int *) shmat (segmento2, 0, 0);
    *p2 = 0;

    int* vetor[] = {p1, p2};

    // PODE SER FEITO COM IF
    // Porém preferi fazer com for porque fica mais clean!
    printf("pid do pai = %d\n", getpid());
    for (int i = 0; i < 2; i++)
    {
        if ((id = fork()) == 0) // proccesso filho
        {
            printf("Executando arquivo auxiliar com filho %d de pid %d\n", i+1, pid);
            execle("ex4aux", vetor[i], NULL, (char *)0);
        }
        else if (id < 0)
        {
            puts ("Erro na criação do novo processo");
            exit (-2);
        }
    }

    while(0 == 0) // processo pai
    {

    }

    // libera a memória compartilhada do processo
    shmdt (p1);
    shmdt (p2);

    // libera a memória compartilhada
    shmctl (segmento1, IPC_RMID, 0);
    shmctl (segmento2, IPC_RMID, 0);
    
    return 0;
}