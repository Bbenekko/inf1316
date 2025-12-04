#include <unistd.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "trabalho.h"
#include "semaforo.h"

#define FIFO_AX_OUT "axFifoOut"

#define WOPENMODE (O_WRONLY | O_NONBLOCK)
#define MAX 5

int main(void)
{
    signal(SIGINT, SIG_IGN);
    pause();

    time_t t;
    srand((unsigned) time(&t));

    char* dirNames[] = {"./subdir1", "./subDir2", "./subdir3", "./subdir4", "./subdir5"};
    char* fileNames[] = {"file1.txt", "file2.txt", "file3.txt", "file4.txt", "file5.txt"};

    int PC = 0;

    __key_t chaveMemIda = 8751;
    __key_t chaveMemResp = 8752;
    __key_t chaveSem = 87511;

    int segmento = shmget (chaveMemIda, sizeof(Info) * 5, S_IRUSR | S_IWUSR);
    if(segmento < 0) {
        perror("shmget");
        exit(1);
    }
    Info* pIda = (Info *) shmat (segmento, 0, 0);
    pIda += 1; //ax2

    int segmento2 = shmget (chaveMemResp, sizeof(Resposta) * 5, S_IRUSR | S_IWUSR);
    if(segmento2 < 0) {
        perror("shmget");
        exit(1);
    }
    Resposta* pResp = (Resposta *) shmat (segmento2, 0, 0);
    pResp += 1; //ax2

    int semId = semget (chaveSem, 1, 0666 | IPC_CREAT);
    printf("SemID: %d\n", semId);

    semaforoP(semId);
    pIda->valorPC = 0;
    pIda->estado = 0; // em andamento
    pIda->pid = getpid();
    pIda->modo = 0;
    pResp->pronto = 0;
    semaforoV(semId);


    while (pIda->valorPC < MAX) {
        int terminou = 0;
        semaforoP(semId);
        pIda->valorPC++;
        pIda->estado = 1;
        pIda->operacao = 'x';
        usleep(100000);  // 0.1 s

        int d = rand()%100 + 1;
        if (d < 15) {
            if(d > 10)
            {
                pIda->dir[0] = 'A';
                pIda->dir[1] = '2';        
                pIda->dir[2] = '\0';
            }
            else
            {
                pIda->dir[0] = 'A';
                pIda->dir[1] = '0';        
                pIda->dir[2] = '\0';
            }

            int chooseDir = rand()%5;
            strcpy(pIda->subdir, dirNames[chooseDir]);


            if (d % 2) // se par manipula arquivo
            {

                 pIda->isFile = 1;
                int chooseFile = rand()%5;
                strcat(pIda->subdir, fileNames[chooseFile]);

                int qtdOffset = rand()%7;
                if (d % 3 == 1) { //escreve no arquivo
                    pIda->operacao = 'w';
                    pIda->offset = qtdOffset * 16;
                } 
                else if (d %3 == 2) { // le arquivo
                    pIda->operacao = 'r';
                    pIda->offset = qtdOffset * 16;

                } 
                else { //remove arquivo 
                    pIda->operacao = 'e';
                }
            }
            
            // senao manipula diretorio
            else 
            {
                if (d % 3 == 1) //cria diretorio
                {
                    pIda->operacao = 'a';
                }
                else if (d % 3 == 2) //lista diretorio
                {
                    pIda->operacao = 'l';
                }
                else // remove diretorio
                {
                    pIda->operacao = 'e';
                }
            }

        } 

        if (pIda->valorPC >= MAX)
        {
            pIda->estado = 2;
        }
        else 
        {
            pIda->estado = 3; // bloqueado
        }

        printf("AX2 - PC: %d, operacao: %c, dir: %s, subdir: %s, offset: %d\n", pIda->valorPC, pIda->operacao, pIda->dir, pIda->subdir, pIda->offset);

        char buf[200];

        semaforoV(semId);
        
        usleep(300000);

        while(!terminou)
        {
            semaforoP(semId);
            if (pResp->pronto == 1)
            {
                printf("AX2 - valorRetorno: %d - Resposta: %s\n", pResp->valorRetorno, pResp->dados);

                terminou = 1;
            }
            semaforoV(semId);
        }
    }
    return 0;
}

