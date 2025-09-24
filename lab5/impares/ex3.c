#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 

#define TAM_VETOR 100   

int main(void)
{
    int fd[2]; /* descritor a ser duplicado */
    int id;

    if (pipe(fd) < 0)
    {
        puts ("Erro ao abrir os pipes");
        exit (-1);
    }

    id = fork();
    if(id == 0) //filho
    {
        close(fd[0]); //tem que fechar leitura
        
        dup2(fd[1], 1);
        system("ps");

        close(fd[1]);
    }
    else if (id > 0)
    {
        close(fd[1]); //tem que fechar escrita

        dup2(fd[0], 0);
        system("wc");
        close(fd[0]);
    }
    else
    {
        puts ("Erro na criação do novo processo\n");
        exit (-2);
    }

    return 0;
} 