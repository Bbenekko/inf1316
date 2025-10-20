#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

#include "fila.h"
#include "kernelSim.h"

//mensagens do controller para o kernel
#ifndef FIFO_KERNEL_IN
#define FIFO_KERNEL_IN "kernelFifoInt" 
#endif

//mensagens do kernel para o controller
#ifndef FIFO_AX_IN  
#define FIFO_AX_IN "axFifoOut"
#endif

#define ROPENMODE (O_RDONLY | O_NONBLOCK)
#define WOPENMODE (O_WRONLY | O_NONBLOCK)

//vetor com Id dos Pids
Info vPids [5];

//filas de processos
Fila* filaD1;
Fila* filaD2;

//arquivos FIFO
int fifoRq, fifoSc;

int pidInterCont = 0;

//pid do processo atual
int indexPidCurrent = -1;
int pauseInt = 0;


unsigned char retornaIndPid(Info vPids[], int pid);
void iniciaVetor(Info vInfos[]);
void configuraFifos(int* fin, int* faX);
void stopHandler(int num);

int main()
{

    signal(SIGINT, stopHandler);
    configuraFifos(&fifoRq, &fifoSc);
    iniciaVetor(vPids);

    filaD1 = criaFila();
    filaD2 = criaFila();

    pidInterCont = fork();

    if(!pidInterCont)
    {
        execle("interControllerSim2", "interControllerSim2", NULL, (char *)0);
        exit(1);
    }
    else{
        kill(pidInterCont, SIGSTOP);
    }

    for(int i = 0; i < 5; i++)
    {
        int pid = fork();
        if(pid == 0) //filho
        {           
            execle("ax2", "ax2", NULL, (char*)0);
            exit(1);            
        }
        else
        {  
            vPids[i].pid = pid;
            kill(pid, SIGSTOP);
        }
    }
    kill(pidInterCont, SIGCONT);


    char buf [200];

    while(1)
    {
        char ch;

        if(pauseInt)
        {
            //fazzer o print 
            for(int i = 0; i < 5; i++)
            {
                kill(vPids[i].pid, SIGSTOP);

            }
            kill(pidInterCont, SIGSTOP);
            pause();
        }
        else{
            for(int i = 0; i < 5; i++)
            {
                if (vPids[i].estado == 1)
                    kill(vPids[i].pid, SIGCONT);
            }
            kill(pidInterCont, SIGCONT);
        }

        if (read (fifoRq, &ch, sizeof(ch)) > 0)
        {
            if(ch == '0')
            {
                if(vPids[indexPidCurrent].estado == 1)
                {
                    kill(vPids[indexPidCurrent].pid, SIGSTOP);
                    vPids[indexPidCurrent].estado = 0;
                    printf("kernel - processo %d bloqueado pelo timeslice\n", vPids[indexPidCurrent].pid);

                }                

                int proxId = (indexPidCurrent + 1) % 5;
                for(int i = 0; i < 5; i++)
                {
                    if(vPids[proxId].estado == 0)
                    {
                        kill(vPids[proxId].pid, SIGCONT);
                        printf("kernel - processo %d liberado pelo timeslice\n", vPids[indexPidCurrent].pid);
                        vPids[proxId].estado = 1;
                        indexPidCurrent = proxId;
                        break;
                    }

                    proxId = (proxId + 1) % 5;
                }                    
            }
            else if (ch == '1')
            {
                int pid = excluiFila(filaD1);
                unsigned char ind = 0;
                if (pid != -1)
                {
                    ind = retornaIndPid(vPids, pid);
                    vPids[ind].estado = 0;
                    vPids[ind].dispositivo = 0;
                    vPids[ind].operacao = 0;
                    printf("kernel - processo %d liberado pelo D1\n", pid);
                }
            }
                
        
            else if(ch == '2')
            {
                int pid = excluiFila(filaD2);
                unsigned char ind = 0;
                if (pid != -1)
                {
                    ind = retornaIndPid(vPids, pid);
                    vPids[ind].estado = 0;
                    vPids[ind].dispositivo = 0;
                    vPids[ind].operacao = 0;
                    printf("kernel - processo %d liberado pelo D2\n", pid);
                }
            }

        }

        if(read (fifoSc, buf, sizeof(buf)) > 0)
        {
            int pid, dispositivo, estado, pc;
            char operacao; 
            int index = 0;
            if(sscanf(buf, "%d %d %d %d %c", &pid, &pc, &dispositivo, &estado, &operacao) == 5)
            {
                index = retornaIndPid(vPids, pid);
                vPids[index].dispositivo = dispositivo;
                vPids[index].valorPC = pc;
                vPids[index].estado = estado;
                vPids[index].operacao = operacao;

                if(dispositivo != 0)
                {
                    kill(pid, SIGSTOP);
                    if(dispositivo == 1)
                    {
                        insereFila(filaD1, pid);
                        vPids[index].qtdVzsD1++;
                        printf("kernel - processo %d bloqueado pelo D1\n", pid);

                    }        
                    else
                    {
                        insereFila(filaD2, pid);
                        vPids[index].qtdVzsD2++;
                        printf("kernel - processo %d bloqueado pelo D2\n", pid);

                    }
                }
            }
        }

    }
}
//realiza a abertura das fifos
void configuraFifos(int* fin, int* faX)
{
    unlink(FIFO_KERNEL_IN);
    if (access(FIFO_KERNEL_IN, F_OK) == -1)
    {
        if (mkfifo (FIFO_KERNEL_IN, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO_KERNEL_IN);
            exit(-1);
        }
        puts ("FIFO criada com sucesso");
    } 

    if ((*fin = open (FIFO_KERNEL_IN, ROPENMODE)) < 0)
    {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO_KERNEL_IN);
        exit(-2);
    }


    unlink(FIFO_AX_IN);
    if (access(FIFO_AX_IN, F_OK) == -1)
    {
        if (mkfifo (FIFO_AX_IN, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO_AX_IN);
            exit(-1);
        }
        puts ("FIFO criada com sucesso");
    } 

    if ((*faX = open (FIFO_AX_IN, ROPENMODE)) < 0)
    {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO_AX_IN);
        exit(-2);
    }
}

void iniciaVetor(Info vInfos[])
{
    for(int i = 0; i < 5; i++)
    {
        vInfos[i].dispositivo = 0;
        vInfos[i].estado = 0;
        vInfos[i].pid = 0;
        vInfos[i].valorPC = 0;
        vInfos[i].operacao = '\0';
        vInfos[i].qtdVzsD1 = 0;
        vInfos[i].qtdVzsD2 = 0;
    }
}

unsigned char retornaIndPid(Info vPids[], int pid)
{
    int j = 0;
    for (;j < 5; j ++)
    {
        if (vPids[j].pid == pid) break;
    }

    return j;
}

void stopHandler(int num)
{
    pauseInt = !pauseInt;
}