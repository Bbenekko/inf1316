#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct bloco 
{
  int valor;
  int seq;
} bloco;

int main (int argc, char *argv[])
{
    int segmento1, segmento2, id, id2, pid, status;
    __key_t chave1 = 8751;
    __key_t chave2 = 8752;

    // aloca a memória compartilhada
    segmento1 = shmget (chave1, sizeof (bloco), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    // associa a memória compartilhada ao processo
    bloco* m1 = (bloco *) shmat (segmento1, 0, 0); // comparar o retorno com -1
    m1->valor = 0;
    m1->seq = 0;


    segmento2 = shmget (chave2, sizeof (bloco), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    bloco* m2 = (bloco *) shmat (segmento2, 0, 0);
    m2->valor = 0;
    m2->seq = 0;

    int valoresAntigos[] = {m1->seq, m2->seq};
    char* chaves[] = {"1", "2"};

    // PODE SER FEITO COM IF
    // Porém preferi fazer com for porque fica mais clean!
    printf("pid do pai = %d\n", getpid());
    for (int i = 0; i < 2; i++)
    {
        if ((id = fork()) == 0) // proccesso filho
        {
            printf("Executando arquivo auxiliar com filho %d de pid %d\n", i+1, getpid());
            execle("ex4aux", chaves[i], NULL, (char *)0);
        }
        else if (id < 0)
        {
            puts ("Erro na criação do novo processo");
            exit (-2);
        }
    }

    while((m1->seq == valoresAntigos[0])|(m2->seq == valoresAntigos[1]))
    {
        sleep(1);
        printf("Processo pai aguardando resultado!\n");
    }

    printf("Resultados:\nm1: %d\nm2: %d\n", m1->valor, m2->valor);


    // libera a memória compartilhada do processo
    shmdt (m1);
    shmdt (m2);

    // libera a memória compartilhada
    shmctl (segmento1, IPC_RMID, 0);
    shmctl (segmento2, IPC_RMID, 0);
    
    return 0;
}