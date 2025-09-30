#include <stdio.h>
#include <sys/stat.h>
int main (void)
{
    if (mkfifo("minhaFifo", S_IRUSR | S_IWUSR) == 0)
    {
        puts ("FIFO criada com sucesso");
        return 0;
    }
    puts ("Erro na criação da FIFO");
    return -1;
}