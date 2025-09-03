#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct bloco 
{
  int valor;
  int seq;
} bloco;

int main(int argc, char *argv[])
{
    int segmento;
    __key_t chave1 = 8751;
    __key_t chave2 = 8752;

    // adicionando randomização "verdadeira"
    time_t t;
    srand((unsigned) time(&t));

    if(strcmp(argv[0], "1"))
    {
        segmento = shmget (chave1, sizeof (bloco), S_IRUSR | S_IWUSR);
        bloco* p = (bloco *) shmat (segmento, 0, 0);

        sleep(rand() % 10 + 2);
        p->valor = rand() % 100 + 1;

        p->seq++;

        shmdt (p);

        printf("Processo de m1 finalizado!\n");
    }
    else if (strcmp(argv[0], "2"))
    {
        segmento = shmget (chave2, sizeof (bloco), S_IRUSR | S_IWUSR);
        bloco* p = (bloco *) shmat (segmento, 0, 0);

        int aux = rand();
        srand(aux); // mudando a seed de random, já que ambos processos são realizados ao mesmo tempo, então sempre tem o mesmo resultado
        sleep(rand() % 10 + 2);
        p->valor = rand() % 100 + 1;

        p->seq++;

        shmdt (p);
        printf("Processo de m2 finalizado!\n");
    }
    else
    {
        puts ("Erro ao adquirir a chave");
        exit (-2);
    }

    return 0;
}