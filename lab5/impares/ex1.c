#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 

#define TAM_VETOR 100   

int main(void)
{
    int fd[2]; /* descritor a ser duplicado */
    int qtdLidos = 0;
    int qtdEnviados = 0;
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
        char frase[TAM_VETOR] = "Ola pai! Aqui e o seu filho. Meu pid e ";
        char idStr [10];
        sprintf(idStr, "%d", getpid());
        strcat(frase, idStr);
        qtdEnviados = write(fd[1], frase, strlen(frase) + 1);
        printf("%d dados enviados\n", qtdEnviados);
        close(fd[1]);
    }
    else if (id > 0)
    {
        close(fd[1]); //tem que fechar escrita
        char frase[TAM_VETOR];

        qtdLidos = read(fd[0], frase, TAM_VETOR);
        printf("%d dados lidos: %s\n", qtdLidos, frase);
        printf("\nAqui e o pai e o pid do filho era %d\n", id);
        close(fd[0]);
    }
    else
    {
        puts ("Erro na criação do novo processo\n");
        exit (-2);
    }

    return 0;
} 