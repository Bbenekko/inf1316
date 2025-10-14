#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

int main(void)
{
    pthread_t threads[2]; // 0 produtor, 1 consumidor

    pthread_join(threads[t],NULL); /* wait for all the threads to terminate*/
}