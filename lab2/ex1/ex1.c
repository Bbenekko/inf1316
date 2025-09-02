#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>


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
    int mv1, mv2, mv3, j, *v1r, *v2r, *v3r, *v1, *v2, *v3, id, status;

    int i; //i -> linha // j -> coluna

    printf("Insira a qtd de linhas: ");
    scanf("%d", &i);
    printf("\n");

    printf("\nInsira a qtd de colunas: ");
    scanf("%d", &j);

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

    printf("Insira os numeros da primeira matriz: ");
    for (int l = 0; l < i * j; l++)
    {
        scanf("%d", v1 + l);
    }
    printf("Insira os numeros da segunda matriz: ");
    for (int l = 0; l < i * j; l++)
    {
        scanf("%d", v2 + l);
    }
    printf("\n");
    printaMatriz(i, j, v1r);       
    printaMatriz(i, j, v2r);    
 
    for(int l = 0; l < i; l++)
    {  
        printf("%p - %p - %p\n", v1, v2, v3);
        if (l != 0)
        {
            v1 += j;
            v2 += j;
            v3 += j;
        }

        printf("%p - %p - %p\n", v1, v2, v3);
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
                printf("%d + %d = %d ---- %p - %p - %p\n", *v1, *v2, *v3, v1, v2, v3);
                v3++;
                v2++;
                v1++;
            }
            exit(2);
        }
        else
        {    
            printf("pai: id do fork: %d // pid proprio: %d // pid do pai: %d\n", id, getpid(), getppid());

            waitpid(id, &status, 0);
            printf("l: %d /// i: %d\n", l, i);
 
            if (l == i - 1)
                printaMatriz(i, j, v3r);   
        }
    }

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