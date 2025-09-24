#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main (int argc, char *argv[])
{
    int nDadosTx, nDadosRx; // quantidade de dados transmitidos/recebidos
    int fd[2]; // descritor dos pipes
    const char textoTX[] = "mensagem";
    char textoRX[sizeof textoTX];
    int pid1, pid2, status;

    if (pipe(fd) < 0)
    {
        puts ("Erro ao abrir os pipes");
        exit (-1);
    }

    if((pid1 = fork()) == 0) // filho 1 (leitor)
    {
        sleep(4);
        close(fd[1]);
        nDadosRx = read(fd[0], textoRX, sizeof textoRX);
        printf("Leitura de %d dados pelo filho 1.\nConteúdo: %s\n", nDadosRx, textoRX);
        close(fd[0]);
        exit(3);
    }
    if ((pid2 = fork()) == 0) // filho 2 (leitor)
    {
        sleep(4);
        close(fd[1]);
        nDadosRx = read(fd[0], textoRX, sizeof textoRX);
        printf("Leitura de %d dados pelo filho 2.\nConteúdo: %s\n", nDadosRx, textoRX);
        close(fd[0]);
        exit(3);
    }

    sleep(2);
    close(fd[0]);
    nDadosTx = write(fd[1], textoTX, strlen(textoTX) + 1);
    close(fd[1]);
    printf("%d dados escritos pelo pai!\n", nDadosTx);

    waitpid(-1, &status, 0);

    
    return 0;
}