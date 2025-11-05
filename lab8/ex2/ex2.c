/* Exemplo de uso de unico semáforo*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>

#define BUFFER_SIZE 16
#define TRUE 1
#define false 0

// cores para output
#define RED  "\x1b[31m"
#define GREEN  "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE  "\x1b[34m"
#define RESET   "\x1b[0m"

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void zeraBuffer(char* buffer);

int main(void)
{
    int segmento, id;
    char* buffer;

    segmento = shmget(IPC_PRIVATE, sizeof(char) * BUFFER_SIZE , IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    buffer = (char *)shmat(segmento, 0, 0);
    zeraBuffer(buffer);

    if ((id = fork()) < 0)
    {
        puts( "Erro na criação do novo processo!");
        exit(-2);
    }
    else if (id == 0) // processo filho - produtor
    {
        while(TRUE)
        {
            for(int i = 0; i < BUFFER_SIZE; i++)
            {
                printf(BLUE"leitor -"RESET" Insira um caracter (apenas um!!!):\n");
                char c = getchar();
                buffer[i] = c;
                printf(BLUE"leitor -"RESET" Caracter inserido na posicao "GREEN"%d"RESET" do buffer!\n", i);
            }
        }
    }
    else //              processo pai - impressor
    {
        while(TRUE)
        {
            if(buffer[BUFFER_SIZE-1] != NULL)
            {
                printf(YELLOW"impressor -"RESET" Entrou em execução...\n");
                printf(YELLOW"impressor -"RESET" Conteúdo do buffer : "GREEN"["RESET);

                for(int i = 0; i < BUFFER_SIZE; i++)
                {
                    printf(GREEN"%c, "RESET, buffer[i]);
                }
                printf(GREEN"]\n"RESET);
                // stuff
                zeraBuffer(buffer);
            }
        }
    }

    return 0;
}

void zeraBuffer(char* buffer)
{
    for(int i = 0; i < BUFFER_SIZE; i++ )
    {
        buffer[i] = NULL;
    }
}