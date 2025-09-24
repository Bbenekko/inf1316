#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main ()
{
    int fdIn, fdOut;
    int retorno; /* valor de retorno de dup */
    int retorno2; 

    if ((fdIn=open("arquivoEnt.txt", O_RDONLY,0666)) == -1)
    {
        perror("Error open()");
        return -1;
    }
    close(0);
    dup(fdIn);

    if ((fdOut=open("arquivoSaida.txt", O_RDWR|O_CREAT|O_TRUNC,0666)) == -1)
    {
        perror("Error open()");
        return -1;
    }
    close(1);
    dup(fdOut);

    char conteudo[1000];
    fgets(conteudo, sizeof conteudo, stdin);
    printf("Conteudo do arquivoEnt: %s", conteudo);
    
    return 0;
}