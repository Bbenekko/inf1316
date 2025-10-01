#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define FIFO "minhaFifo"
#define ROPENMODE (O_RDONLY | O_NONBLOCK)
#define WOPENMODE (O_WRONLY | O_NONBLOCK)

int main (void)
{
    int id = 0;
    char vPalavra[][10] = {"Linux", "Windows"};
    int status;
    int fifo;
    char vRetorno[2][10];
    char ch;
    int i = 0, j = 0;

    if (access(FIFO, F_OK) == -1)
    {
        if (mkfifo (FIFO, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO);
            return -1;
        }
        puts ("FIFO criada com sucesso");
    }  

    if ((fifo = open (FIFO, ROPENMODE)) < 0)
    {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO);
        return -2;
    } 

    for (int i = 0; i < 2; i++)
    {
        id = fork();
        if(id == 0) //filho
        {
            if ((fifo = open(FIFO, WOPENMODE)) < 0)
            {
                puts ("Erro ao abrir a FIFO para escrita"); 
                return -1;
            }
            printf("palavra[%d]: %s --- %ld\n", i, vPalavra[i], strlen(vPalavra[i]) + 1);
            write(fifo, vPalavra[i],strlen(vPalavra[i]) + 1);
            close (fifo);
            exit(0);
        }
        if(id < 0)
        {
            puts ("Erro ao abrir os pipes");
            exit (-1);
        }

    }

    waitpid(-1, &status, 0);

    while (read (fifo, &ch, sizeof(ch)) > 0)
    {
        vRetorno[i][j++] = ch;

        if (ch == '\0')
        {
            i++;
            j = 0;
        }
    }

    close(fifo);

    printf("Palavras lidas: %s e %s\n", vRetorno[0], vRetorno[1]);
    return 0;
} 