#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define NUM_THREADS 5
#define PULO_MAXIMO 100
#define DESCANSO_MAXIMO 1
#define DISTANCIA_PARA_CORRER 100
static int classificacao = 1;
static pthread_mutex_t lock;
static char * resp[200];
static int cont = 0;

void *Correr(void *sapo)
{
    int pulos = 0;
    int distanciaJaCorrida = 0;
    while (distanciaJaCorrida <= DISTANCIA_PARA_CORRER) 
    {
        int pulo = rand() % PULO_MAXIMO;
        distanciaJaCorrida += pulo;
        pulos++;
        printf("Sapo %d pulou\n", (int) sapo);
        int descanso = rand() % DESCANSO_MAXIMO;
        sleep(descanso);
    }
    pthread_mutex_lock( &lock); 
    printf("Sapo %d chegou na posicaoo %d com %d pulos\n", (int) sapo,
    classificacao, pulos);
    cont++;
    pthread_mutex_unlock( &lock );
    classificacao++;
    pthread_exit(NULL);
}

int main()
{
    classificacao =1;
    pthread_t threads[NUM_THREADS];
    int t;
    printf("Corrida iniciada ... \n");
    for(t=0;t < NUM_THREADS;t++) pthread_create(&threads[t], NULL, Correr, (void *) t);
    for(t=0;t < NUM_THREADS; t++) pthread_join(threads[t],NULL);
    printf("\n Acabou!!\n");
    pthread_exit(NULL);

}