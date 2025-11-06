#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>

#define BUFFER_SIZE 16
#define TRUE 1
#define false 0

// cores para output
#define RED  "\x1b[31m"
#define GREEN  "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE  "\x1b[34m"
#define RESET   "\x1b[0m"

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// inicializa o valor do semáforo
int setSemValue(int vecSemId, int semId, int value);
// remove o semáforo
void delSemValue(int semId);
// operação P
int semaforoP(int vecSemId, int semId);
// operação V
int semaforoV(int vecSemId, int semId);

void zeraBuffer(char* buffer);

int main(void)
{
    int segmento, id, vecSemId;
    int semPro = 0;
    int semImp = 1;
    char* buffer;

    segmento = shmget(IPC_PRIVATE, sizeof(char) * BUFFER_SIZE , IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    buffer = (char *)shmat(segmento, 0, 0);
    zeraBuffer(buffer);

    vecSemId = semget(8751, 2, 0666 | IPC_CREAT);
    if (vecSemId == -1)
    {
        puts( "Erro na criação do  semáforo!");
        exit(-1);
    }
    
    setSemValue(vecSemId, semPro, 1);
    setSemValue(vecSemId, semImp, 0);

    if ((id = fork()) < 0)
    {
        puts( "Erro na criação do novo processo!");
        exit(-2);
    }
    else if (id == 0) // processo filho - produtor
    {
        char c;
        while(TRUE)
        {
            semaforoP(vecSemId, semPro);
            for(int i = 0; i < BUFFER_SIZE; i++)
            {
                printf(BLUE"leitor -"RESET" Insira um caracter (apenas um!!!) na posição " GREEN"%d"RESET":\n", i);
                c = getchar();
                while(getchar() != '\n'); // consome lixo (e o enter) após o caracter!
                buffer[i] = c;
            }
            semaforoV(vecSemId, semImp);
        }
    }
    else //              processo pai - impressor
    {
        while(TRUE)
        {
            semaforoP(vecSemId, semImp);

            printf(YELLOW"impressor -"RESET" Entrou em execução...\n");
            printf(YELLOW"impressor -"RESET" Conteúdo do buffer : "GREEN"["RESET);


            for(int i = 0; i < BUFFER_SIZE; i++)
            {
                printf(GREEN"%c"RESET, buffer[i]);
                if (i < BUFFER_SIZE-1) printf(", ");
            }
            printf(GREEN"]\n"RESET);

            zeraBuffer(buffer);

            semaforoV(vecSemId, semPro);
        }
    }

    return 0;
}

void zeraBuffer(char* buffer)
{
    for(int i = 0; i < BUFFER_SIZE; i++ )
    {
        buffer[i] = '\0';
    }
}

int setSemValue(int vecSemId, int semId, int value)
{
    union semun semUnion;
    semUnion.val = value;
    return semctl(vecSemId, semId, SETVAL, semUnion);
}

void delSemValue(int semId)
{
    union semun semUnion;
    semctl(semId, 0, IPC_RMID, semUnion);
}

int semaforoP(int vecSemId, int semId)
{
    struct sembuf semB;
    semB.sem_num = semId;
    semB.sem_op = -1;
    semB.sem_flg = SEM_UNDO;
    semop(vecSemId, &semB, 1);
    return 0;
}

int semaforoV(int vecSemId, int semId)
{
    struct sembuf semB;
    semB.sem_num = semId;
    semB.sem_op = 1;
    semB.sem_flg = SEM_UNDO;
    semop(vecSemId, &semB, 1);
    return 0;
}