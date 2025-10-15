#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define FIFO_SERVER_IN "serverFifoIn"
#define FIFO_SERVER_OUT "serverFifoOut"

int inFIFO, outFIFO;

int main (void)
{
    char ch;
    if (access(FIFO_SERVER_IN, F_OK) == -1)
    {
        if (mkfifo (FIFO_SERVER_IN, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO_SERVER_IN);
            return -1;
        }
    }
    puts ("Abrindo FIFO de entrada do cliente!");
    if ((inFIFO = open (FIFO_SERVER_IN, O_WRONLY)) < 0)
    {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO_SERVER_IN);
        return -2;
    }

    if (access(FIFO_SERVER_OUT, F_OK) == -1)
    {
        if (mkfifo (FIFO_SERVER_OUT, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO_SERVER_OUT);
            return -1;
        }
    }
    puts ("Abrindo FIFO de entrada do cliente!");
    if ((outFIFO = open (FIFO_SERVER_OUT, O_RDONLY)) < 0)
    {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO_SERVER_OUT);
        return -2;
    }

    for(;;)
    {
        char mensagem[1000];
        puts ("Aguardando entrada!");
        fgets(mensagem, sizeof mensagem, stdin);
        puts ("Fim da entrada!");
        
        write(inFIFO, mensagem, strlen(mensagem));
        puts ("Aguardando leitura!");
        while (read (outFIFO, &ch, sizeof(ch)) == 1 && ch != '\n') putchar (ch);
        putchar('\n');
        puts ("Fim da leitura");
    }



    return 0;   
}