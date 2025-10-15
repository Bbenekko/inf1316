#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define MAX_PRODUZ 64
#define MAXFILA 8
#define NUM_THREADS 2 // possui 2 threads

// cores para output
#define RED  "\x1b[31m"
#define YELLOW  "\x1b[33m"
#define BLUE  "\x1b[34m"
#define RESET   "\x1b[0m"

pthread_mutex_t mutex; // mutex semaphore 
pthread_cond_t consumer_c, producer_c; // 2 condition variables for consumer, and producer

int fila[MAXFILA]; // fila de 8 posições
int vaziosFila = MAXFILA;

void* producer(void *ptr)
{
    for (int i = 1; i <= (MAX_PRODUZ/NUM_THREADS); i++) {
        sleep(1);
        pthread_mutex_lock(&mutex); // entrando zona critica
        while(vaziosFila == 0) // espera caso a fila esteja cheia
        {
            printf(RED"Fila cheia!\n"RESET);
            pthread_cond_wait(&producer_c, &mutex);
        }

        fila[MAXFILA-vaziosFila] = rand() + 1;
        printf(BLUE"Conteudo inserido na fila: %d\n"RESET, fila[MAXFILA-vaziosFila]);
        vaziosFila--;

        pthread_cond_signal(&consumer_c); /* wake up consumer */
        pthread_mutex_unlock(&mutex); /* release the buffer */
    }
    pthread_exit(0);
}

void* consumer(void *ptr) 
{
    for (int i = 1; i <= (MAX_PRODUZ/NUM_THREADS); i++) 
    {
        sleep(2);
        pthread_mutex_lock(&mutex); // entrando na zona critica
        while (vaziosFila == MAXFILA) // espera caso a fila esteja vazia
        {
            printf(RED"Fila vazia!\n"RESET);
            pthread_cond_wait(&consumer_c, &mutex);
        }

        printf(YELLOW"Conteudo lido da primeira posição da fila: %d\n"RESET, fila[0]);
        fila[0] = 0; /* consume the item */
        vaziosFila++;

        for(int j = 0; j < (MAXFILA - 1); j++) fila[j] = fila[j+1];

        pthread_cond_signal(&producer_c); /* wake up producer*/
        pthread_mutex_unlock(&mutex); /* release the buffer */
    }
    pthread_exit(0);
}

int main() 
{
    pthread_t pro, con, pro2, con2;

    // Initialize the mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&consumer_c, NULL); /* Initialize consumer cond variable */
    pthread_cond_init(&producer_c, NULL); /* Initialize producer cond variable */

    // Create the threads
    pthread_create(&con, NULL, consumer, NULL);
    pthread_create(&pro, NULL, producer, NULL);
    pthread_create(&con2, NULL, consumer, NULL);
    pthread_create(&pro2, NULL, producer, NULL);

    pthread_join(con, NULL);
    pthread_join(pro, NULL);
    pthread_join(con2, NULL);
    pthread_join(pro2, NULL);
}