#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};
// inicializa o valor do semáforo
int setSemValue(int semId);
// remove o semáforo
void delSemValue(int semId);
// operação P
int semaforoP(int semId);
//operação V
int semaforoV(int semId);

int main(void)
{
    printf("Aux1 - Entrei no filho %d\n", getpid());
    __key_t chave1 = 8751;
    __key_t chave2 = 8753;


    int segmento = shmget (chave1, sizeof(int), S_IRUSR | S_IWUSR);
    if(segmento < 0) {
        perror("shmget");
        exit(1);
    }
    int* p = (int *) shmat (segmento, 0, 0);

    int semId = semget (chave2, 1, 0666 | IPC_CREAT);
    printf("SemID: %d\n", semId);
    for(int i = 0; i < 10; i++) {
        semaforoP(semId);
        (*p)++;
        printf("Valor atual: %d --- filho: %d\n", *p, getpid());
        semaforoV(semId);
        sleep(1);
    }
    shmdt (p);
    shmctl (segmento, IPC_RMID, 0);
    printf ("\nProcesso %d terminou\n", getpid());
    exit(0);
}

int setSemValue(int semId)
{
    union semun semUnion;
    semUnion.val = 1;
    return semctl(semId, 0, SETVAL, semUnion);
}
void delSemValue(int semId)
{
    union semun semUnion;
    semctl(semId, 0, IPC_RMID, semUnion);
}
int semaforoP(int semId)
{
    struct sembuf semB;
    semB.sem_num = 0;
    semB.sem_op = -1;
    semB.sem_flg = SEM_UNDO;
    semop(semId, &semB, 1);
    return 0;
}
int semaforoV(int semId)
{
    struct sembuf semB;
    semB.sem_num = 0;
    semB.sem_op = 1;
    semB.sem_flg = SEM_UNDO;
    semop(semId, &semB, 1);
    return 0;
}