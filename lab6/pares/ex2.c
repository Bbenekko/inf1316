#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO "minhaFifo"
int main (void)
{
    int id = 0;
    char vPalavra[] = {"Linux", "Windows"};
    int status;

    if (mkfifo(FIFO, S_IRUSR | S_IWUSR) == 0)
    {
        puts ("FIFO criada com sucesso");
    }
    else{
        puts ("Erro na criação da FIFO");
        return -1;
    }

    


    for (int i = 0; i < 2; i++)
    {
        id = fork();
        if(id == 0) //filho
        {
            int fifo;
            if ((fifo = open("minhaFifo", "w")) < 0)
            {
                puts ("Erro ao abrir a FIFO para escrita"); return -1;
            }
            write(fifo, vPalavra[i],strlen(vPalavra[i] + 1));
            close (fifo);
            exit(0);
        }
        if(id < 0)
        {
            puts ("Erro ao abrir os pipes");
            exit (-1);
        }

    }

    wait(&status);



} 