#include <stdio.h>
#include <unistd.h>
#include <signal.h>
void handler(int sinal);
int main (void)
{
signal(SIGUSR1, handler);
puts ("vou parar...");
pause();
puts ("Continuei!");
return 0;
}
void handler(int sinal)
{
printf ("Sinal %d recebido\n", sinal);
}