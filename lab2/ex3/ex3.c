#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    int segmento, segmento2, *v, *vr, *ind, id, status, tamVetor= 20;

    // aloca a memória compartilhada
    segmento = shmget (IPC_PRIVATE, sizeof (int) * tamVetor, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    segmento2 = shmget (IPC_PRIVATE, sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    // associa a memória compartilhada ao processo
    v = (int *) shmat (segmento, 0, 0); // comparar o retorno com -1
    ind = (int *) shmat (segmento2, 0, 0); // comparar o retorno com -1
    *ind = -1;
    v[0] = 9; v[1] = 1; v[2] = 15; v[3] = 18; v[4] = 3; v[5] = 13; v[6] = 4; v[7] = 19; v[8] = 20; v[9] = 2;
    v[10] = 7; v[11] = 14; v[12] = 6; v[13] = 11; v[14] = 10; v[15] = 5; v[16] = 8; v[17] = 12; v[18] = 17; v[19] = 16;

    const int qtdSegmentos = 5;

    const int key = 12;

    vr = v;

    for(int i = 0; i < qtdSegmentos; i++)
    {
        if (i > 0)
            v+=4;
        if ((id = fork()) < 0)
        {
            puts ("Erro na criação do novo processo\n");
            exit (-2);
        }
        else if (id == 0)
        {
            printf(" filho: id do fork: %d // pid proprio: %d // pid do pai: %d\n", id, getpid(), getppid());
            for(int j = 0; j < tamVetor/qtdSegmentos; j++)
            {
                if (*(v+j) == key)
                {
                    *ind = i * 4 + j;
                    printf("Pid do filho que achou a chave: %d\n", getpid());
                }

            }
            exit(0);
        }
    }
    for (int i = 0; i < qtdSegmentos; i++)
    {
        wait(&status);
        printf ("Processo pai -- ind da chave = %d\n", *ind);
    }

    // libera a memória compartilhada do processo
    shmdt (vr);
    shmdt (ind);

    // libera a memória compartilhada
    shmctl (segmento, IPC_RMID, 0);
    shmctl (segmento2, IPC_RMID, 0);
    
    return 0;
}