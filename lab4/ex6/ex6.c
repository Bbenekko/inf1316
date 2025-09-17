#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

clock_t inicio, final;
short ligacaoTerminada = 0;

void inicioHandler(int sinal)
{
    printf("Ligação iniciada!\n");
    inicio = clock();
}

void terminoHandler(int sinal)
{
    printf("Ligação terminada!\n");
    final = clock();
    ligacaoTerminada = 1;
}

int main(void)
{
    int tempo;
    float valor = 0;

    void (*p)(int);

    p = signal(SIGUSR1, inicioHandler);

    p = signal(SIGUSR2, terminoHandler);

    while(ligacaoTerminada == 0);

    tempo = ((int) (final - inicio)) / CLOCKS_PER_SEC;
    if (tempo <= 60)
    {
        valor = tempo * 0.02;
    }
    else
    {
        valor = 1.2 + ((tempo-60)* 0.01);
    }
    
    printf("Ligação de %d minutos e %d segundos!\nCusto da ligação: R$%0.2f\n", tempo/60, tempo%60, valor);

    return 0;
}