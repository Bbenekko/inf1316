#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
int main (int argc, char *argv[])
{
int nDadosTx, nDadosRx; // quantidade de dados transmitidos/recebidos
int fd[2]; // descritor dos pipes
const char textoTX[] = "uma mensagem";
char textoRX[sizeof textoTX];
if (pipe(fd) < 0)
{
puts ("Erro ao abrir os pipes");
exit (-1);
}
nDadosTx = write(fd[1], textoTX, strlen(textoTX)+1);
printf("%d dados escritos\n", nDadosTx);
nDadosRx = read(fd[0], textoRX, sizeof textoRX);
printf("%d dados lidos: %s\n", nDadosRx, textoRX);
close(fd[0]); close(fd[1]);
return 0;
}