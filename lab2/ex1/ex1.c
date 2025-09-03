#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

#define MANUAL 0

void printaMatriz(int i, int j, int* m)
{
    for (int k = 0; k < i; k++)
    {
        for(int l = 0; l < j; l++)
        {
            printf("%d ", *m);
            m++;
        }
        printf("\n");
    }
    printf("\n");
}


int main (int argc, char *argv[])
{
    int mv1, mv2, mv3, *v1r, *v2r, *v3r, *v1, *v2, *v3, id, status;

    int i, j; //i -> linha // j -> coluna 

    #if MANUAL == 1  
    printf("Insira a qtd de linhas: ");
    scanf("%d", &i);
    printf("\n");

    printf("\nInsira a qtd de colunas: ");
    scanf("%d", &j); 
    #else
    int v1p[] = {5,7,9,6,3,6,3,1,2};
    int v2p[] = {5,3,0,6,2,6,5,7,0};
    i = 3;
    j = 3;
    #endif


    mv1 = shmget (IPC_PRIVATE, sizeof (int) * i * j, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWGRP);
    mv2 = shmget (IPC_PRIVATE, sizeof (int) * i * j, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWGRP);
    mv3 = shmget (IPC_PRIVATE, sizeof (int) * i * j, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWGRP);

    // associa a memória compartilhada ao processo
    v1 = (int *) shmat (mv1, 0, 0); // comparar o retorno com -1
    v2 = (int *) shmat (mv2, 0, 0); // comparar o retorno com -1
    v3 = (int *) shmat (mv3, 0, 0); // comparar o retorno com -1

    v1r = v1;
    v2r = v2;
    v3r = v3;    

    #if MANUAL == 1  
    printf("Insira os numeros da primeira matriz: ");
    #endif
    for (int l = 0; l < i * j; l++)
    {
        #if MANUAL == 1  
        scanf("%d", v1 + l);
        #else
        v1[l] = v1p[l];
        #endif
    }

    #if MANUAL == 1  
    printf("Insira os numeros da segunda matriz: ");
    #endif
    for (int l = 0; l < i * j; l++)
    {
        #if MANUAL == 1  
        scanf("%d", v2 + l);
        #else
        v2[l] = v2p[l];
        #endif
    }
    printf("\n");
    printaMatriz(i, j, v1r);       
    printaMatriz(i, j, v2r);    
 
    for(int l = 0; l < i; l++)
    {
        if (l != 0)
        {
            v1 += j;
            v2 += j;
            v3 += j;
        }
        id = fork();   
        if (id < 0)
        {
            puts ("Erro na criação do novo processo");
            exit (-2);
        }
        else if (id == 0)
        {
            printf(" filho: id do fork: %d // pid proprio: %d // pid do pai: %d\n", id, getpid(), getppid());
            for(int m = 0; m < j; m++)
            {
                *v3 = *v1 + *v2;
                printf("%d + %d = %d \n", *v1, *v2, *v3);
                v3++;
                v2++;
                v1++;
            }
            printf("\n");
            exit(0);
        }
    }

    for(int l = 0; l < i; l++)
    {    
        wait(&status);             
    }
    printf("pai: pid proprio: %d \n", getpid());
    printaMatriz(i, j, v3r);  
    // libera a memória compartilhada do processo
    shmdt (v1r);
    shmdt (v2r);
    shmdt (v3r);

    // libera a memória compartilhada
    shmctl (mv1, IPC_RMID, 0);
    shmctl (mv2, IPC_RMID, 0);
    shmctl (mv3, IPC_RMID, 0);
    
    return 0;
}