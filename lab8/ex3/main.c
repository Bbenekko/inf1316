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
    __key_t chave1 = 8751;
    __key_t chave2 = 8753;
    int vPids[2];
    char* vExec[] = {"aux2", "aux1"};


    int segmento = shmget (chave1, sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(segmento < 0) {
        perror("shmget");
        exit(1);
    }
    int* p = (int *) shmat (segmento, 0, 0);

    int semId = semget (chave2, 1, 0666 | IPC_CREAT);
    printf("SemID: %d\n", semId);
    int id, status;
    setSemValue(semId);
    for (int i = 0; i < 2; i++)
    {
        printf("i: %d\n", i);
        vPids[i] = fork();
        if (vPids[i] == 0) //processo filho
        {   
            printf("id do filho: %d\n", getpid());
            execl(vExec[i], vExec[i], NULL, (char *)0);
        }
        else if( vPids[i] < 0 )
        {
            puts ("Erro na criação do novo processo");
            exit (-2);
        }
    }
    waitpid(vPids[0], &status, 0);
    waitpid(vPids[1], &status, 0);
    printf("Processo pai %d terminou\n", getpid());
    delSemValue(semId);
    shmdt (p);
    shmctl (segmento, IPC_RMID, 0);
    return 0;
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