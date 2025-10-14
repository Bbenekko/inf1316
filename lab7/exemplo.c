#include <pthread.h>
#include <stdio.h>
#define NUM_THREADS 5
void *PrintHello(void *threadid)
{
printf("\n%d: Hello World!\n", threadid);
/* do other things */
pthread_exit(NULL); /*not necessary*/
}
int main()
{
pthread_t threads[NUM_THREADS];
int t;
for(t=0;t < NUM_THREADS;t++)
{
printf("Creating thread %d\n", t);
pthread_create(&threads[t], NULL, PrintHello, (void *)t);
}
for(t=0; t < NUM_THREADS; t++)
pthread_join(threads[t],NULL); /* wait for all the threads to
terminate*/
}