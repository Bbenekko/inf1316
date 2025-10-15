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
    char mensagem[1000];
    if (access(FIFO_SERVER_IN, F_OK) == -1)
    {
        if (mkfifo (FIFO_SERVER_IN, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO_SERVER_IN);
            return -1;
        }
    }
    puts ("Abrindo FIFO de entrada do servidor!");
    if ((inFIFO = open (FIFO_SERVER_IN, O_RDONLY)) < 0)
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
    puts ("Abrindo FIFO de entrada do servidor!");
    if ((outFIFO = open (FIFO_SERVER_OUT, O_WRONLY)) < 0)
    {   
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO_SERVER_OUT);
        return -2;
    }

    for(;;)
    {
        int n = 0;
        puts ("Aguardando leitura!");
        while (n < (int)sizeof(mensagem)-1 && read (inFIFO, &ch, sizeof(ch)) > 0 && ch != '\n') 
        {
            mensagem[n] = ch;
            n++;
        }
        mensagem[n] = '\0';
        printf("Mensagem recebida: %s\n", mensagem);
        for(int i = 0; i < strlen(mensagem); i++)
        {
            if (mensagem[i] >= 'a' && mensagem[i] <= 'z') mensagem[i] = mensagem[i] - 32;
        }
        write(outFIFO, mensagem, n);
        write(outFIFO, "\n", 1);
    }



    return 0;
}