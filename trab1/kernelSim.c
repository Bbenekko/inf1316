#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "kernelSim.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#include "fila.h"

//mensagens do controller para o kernel
#ifndef FIFO_KERNEL_IN
#define FIFO_KERNEL_IN "kernelFifoInt" 
#endif

//mensagens do kernel para o controller
#ifndef FIFO_KERNEL_OUT  
#define FIFO_KERNEL_OUT "kernelFifoOut"
#endif

#define ROPENMODE (O_RDONLY | O_NONBLOCK)
#define WOPENMODE (O_WRONLY | O_NONBLOCK)

void configuraFifos(int* fin, int* fout);
unsigned char retornaIndPid(int vPids[], int pid);
void atualizaVetor(Info vInfos[], Info infoNova, int vPids[], int pid);
void iniciaVetor(Info vInfos[]);

//pid do processo atual
static int pid;

//vetor com Id dos Pids
static int vPids [5];
    
//vetor que guarda a quantidade de que cada processo foi parado
static int vQtdParado [5];

//vetor que que grauda o endereco do vetor de informcacoes na memoria compartilhada
static int *vInfoComp;

//vetor que contem as pipes para os processos filhos - servem para receber o PC
int* vPipes [5][2];

//filas de processos
Fila* filaProntos;
Fila* filaD1;
Fila* filaD2;


int main(void)
{    
    filaProntos = criaFila();
    filaD1 = criaFila();
    filaD2 = criaFila();

    //booleano para saber se é o primeiro filho que será o interController
    unsigned char ePrimeiro = 1;

    //arquivos FIFO
    int fifoIn, fifoOut;

    //saber se foram enviados 1 ou 1 
    int foramDuasInterrop = 0;

    //area da memoria compartilhada para compartilhar os status dos processos
    int mv = shmget (IPC_PRIVATE, sizeof (Info) * 5, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWGRP);
    vInfoComp = (Info *) shmat (mv, 0, 0);
    iniciaVetor(vInfoComp);
    
    //vetor que guardara a string do valor do endereco da memoria compartilhada
    char vEnvio[100];

    //booleano para saber se houver chamada de sistema ou não durante o processo: 0 - nao teve; 1 - teve
    unsigned char teveChamada = 0;

    configuraFifos(fifoIn, fifoOut);

    int* vPipesU = vPipes;

    //for para criacao dos filhos
    for(int i = 0; i < 6; i++)
    {
        pid = fork();
        if(!ePrimeiro) //para poder colocar as informacoes nos vetores depois de já ter criado o interControllerSim
        {
            vPids[i-1] = pid;
            pipe(vPipes[i-1]);
            vPipesU = vPipes[i-1];
        } 
        if(pid == 0) //filho
        {
            if (ePrimeiro)
            {
                execle("interControllerSim", "interControllerSim", NULL, (char *)0);
            }
            else
            {
                close(*vPipesU);
                dup2(*vPipesU + 1, 1);
                execle("ax", "ax", NULL, (char*)0);
            }
        }
    }

    for(int i = 0; i < 5; i++)
    {
        close(vPipes[i][1]);
        kill(vPids[i], SIGSTOP);
        insereFila(filaProntos, vPids[i]);
    }

    //escrever na fifo o endereco da memmoria compartilhada que esta o vetor de informacoes
    sprintf(vEnvio, "%d", vInfoComp);
    write(fifoOut, vEnvio, strlen(vInfoComp) + 1);

    ePrimeiro = 1;

    while(1)
    {
        char vResposta [3];
        char ch;
        unsigned int i = 0;
        Info infoNova;
        char frase[10];

        if(ePrimeiro)
        {
            ePrimeiro = 0;
        }
        else if (!teveChamada)
        {
            kill(pid, SIGSTOP);
            insereFila(filaProntos, pid);

            //atualiza vetor de infos na memoria para o processo atual apos ele ser pausado por fim de tempo
            infoNova.estado = 0;
            infoNova.dispositivo = 0;
            infoNova.operacao = '\0';
            infoNova.estaTerminado =  0;

            int j = retornaIndPid(vPids, pid);

            vQtdParado[j]++;
            infoNova.qtdVezesParado = vQtdParado[j];

            char frase[10];
            read(vPipes[j][0], frase, 10);
            int pc = atoi(frase);
            infoNova.valorPC = pc;
            fprintf(stderr, "PC atual: %d", pc);


            atualizaVetor(vInfoComp, infoNova, vPids, pid);
        }

        while (read (fifoIn, &ch, sizeof(ch)) > 0)
        {
            vResposta[i] = ch;
        }

        if(vResposta == '0')
        {
            if (!foramDuasInterrop)
                pid = excluiFila(filaProntos);
            else
            {
                pid = excluiFila(filaD2);
                foramDuasInterrop = 0;
            }
                
        }
        else if (vResposta[i] == "01")
            pid = excluiFila(filaD1);
     
        else if(vResposta == "02")
            pid = excluiFila(filaD2);

        else if(vResposta == "012")
            pid = excluiFila(filaD1);

        if (pid == -1)
            pid = excluiFila(filaProntos);

        kill(pid, SIGCONT);

        //atualiza vetor de infos na memoria para o processo atual apos continuar
        infoNova.estado = 1;
        infoNova.dispositivo = 0;
        infoNova.operacao = '\0';
        infoNova.estaTerminado =  0;

        int j = retornaIndPid(vPids, pid);
        infoNova.qtdVezesParado = vQtdParado[j];


        read(vPipes[j][0], frase, 10);
        int pc = atoi(frase);
        infoNova.valorPC = pc;
        fprintf(stderr, "PC atual: %d", pc);


        atualizaVetor(vInfoComp, infoNova, vPids, pid);  

        usleep(500000000);

    }

    return 0;
}

//realiza a abertura das fifos
void configuraFifos(int* fin, int* fout)
{
  if (access(FIFO_KERNEL_IN, F_OK) == -1)
    {
        if (mkfifo (FIFO_KERNEL_IN, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO_KERNEL_IN);
            return -1;
        }
        puts ("FIFO criada com sucesso");
    } 

    if ((*fin = open (FIFO_KERNEL_IN, ROPENMODE)) < 0)
    {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO_KERNEL_IN);
        return -2;
    }

    if (access(FIFO_KERNEL_OUT, F_OK) == -1)
    {
        if (mkfifo (FIFO_KERNEL_OUT, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO_KERNEL_OUT);
            return -1;
        }
        puts ("FIFO criada com sucesso");
    }    

     if ((*fout = open (FIFO_KERNEL_OUT, WOPENMODE)) < 0)
    {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO_KERNEL_OUT);
        return -2;
    }
}

//inicia vetor de informacoes da memoria compartilhada
void iniciaVetor(Info vInfos[])
{
    for(int i = 0; i < 5; i++)
    {
        vInfos[i].dispositivo = 0;
        vInfos[i].estado = 0;
        vInfos[i].estaTerminado = 0;
        vInfos[i].qtdVezesParado = 0;
        vInfos[i].valorPC = 0;
    }
}

void atualizaVetor(Info vInfos[], Info infoNova, int vPids[], int pid)
{
    int i = 0;
    for (;i < 5; i ++)
    {
        if (vPids[i] == pid) break;
    }

    vInfos[i].dispositivo = infoNova.dispositivo;
    vInfos[i].estado = infoNova.estado;
    vInfos[i].estaTerminado = infoNova.estaTerminado;
    vInfos[i].qtdVezesParado = infoNova.qtdVezesParado;
    vInfos[i].valorPC = infoNova.valorPC;
}

unsigned char retornaIndPid(int vPids[], int pid)
{
    int j = 0;
    for (;j < 5; j ++)
    {
        if (vPids[j] == pid) break;
    }

    return j;
}

void syscall(int dispositivo, char operacao)
{
    kill(pid, SIGSTOP);

    if(dispositivo == 1)
        insereFila(filaD1, pid);
    
    else
        insereFila(filaD2, pid);

    Info infoNova;
    char frase[10];

    infoNova.estado = 0;
    infoNova.dispositivo = dispositivo;
    infoNova.operacao = operacao;
    infoNova.estaTerminado =  0;

    int j = retornaIndPid(vPids, pid);
    vQtdParado[j]++;
    infoNova.qtdVezesParado = vQtdParado[j];

    read(vPipes[j][0], frase, 10);
    int pc = atoi(frase);
    infoNova.valorPC = pc;

    fprintf(stderr, "PC atual: %d", pc);

    atualizaVetor(vInfoComp, infoNova, vPids, pid); 
}