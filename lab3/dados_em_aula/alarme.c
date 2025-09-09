#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#define TEMPO 10
#define EVER ;;
void trataAlarme(int sinal);
int main (void)
{
signal (SIGALRM, trataAlarme);
alarm (TEMPO);
for(EVER);
return 0;
}
void trataAlarme(int sinal)
{
printf ("%d segundos\n", TEMPO);
alarm (TEMPO);
}
