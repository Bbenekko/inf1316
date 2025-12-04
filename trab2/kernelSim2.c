#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

#include "fila.h"
#include "trabalho.h"

//mensagens do controller para o kernel
#ifndef FIFO_KERNEL_IN
#define FIFO_KERNEL_IN "kernelFifoInt" 
#endif

//mensagens da kernel para o controller
#ifndef FIFO_KERNEL_OUT
#define FIFO_KERNEL_OUT "kernelFifoOut"
#endif

//mensagens dos processos para o kernel
#ifndef FIFO_AX_IN  
#define FIFO_AX_IN "axFifoOut"
#endif

#define ROPENMODE (O_RDONLY)
#define WOPENMODE (O_WRONLY)

//#define ROPENMODE (O_RDONLY | O_NONBLOCK)
//#define WOPENMODE (O_WRONLY | O_NONBLOCK)

//vetor com Id dos Pids
Info vPids [5];

//filas de processos
Fila* filaD1;
Fila* filaD2;

//arquivos FIFO
int fifoRq, fifoSc, fifoOut;

int pidInterCont = 0;

//pid do processo atual
int indexPidCurrent = -1;
int pauseInt = 0;


unsigned char retornaIndPid(Info vPids[], int pid);
void iniciaVetor(Info vInfos[]);
void configuraFifos(int* fin, int* faX, int* fout);
void stopHandler(int num);

int main()
{

    signal(SIGINT, stopHandler);
    iniciaVetor(vPids);

    filaD1 = criaFila();
    filaD2 = criaFila();

    __key_t chaveMemVinda= 8751;
    __key_t chaveMemResp = 8752;
    __key_t chaveSem = 87511;

    int segmento = shmget (chaveMemVinda, sizeof(Info) * 5, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(segmento < 0) {
        perror("shmget");
        exit(1);
    }
    Info* pIda = (Info *) shmat (segmento, 0, 0);
    

    int segmento2 = shmget (chaveMemResp, sizeof(Resposta) * 5, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(segmento2 < 0) {
        perror("shmget");
        exit(1);
    }
    Resposta* pResp = (Resposta *) shmat (segmento2, 0, 0);

    int semId = semget (chaveSem, 1, 0666 | IPC_CREAT);
    printf("SemID: %d\n", semId);

    char* nameExecs[] = {"ax1", "ax2", "ax3", "ax4", "ax5"};
    char** ptrNameExecs = nameExecs;

     for(int i = 0; i < 5; i++)
    {
        ptrNameExecs = nameExecs + i;
        int pid = fork();
        if(pid == 0) //filho
        {           
            printf("Executando filho!\n");
            execle(ptrNameExecs, ptrNameExecs, NULL, (char*)0);
            exit(1);            
        }
        else
        {  
            pIda[i].pid = pid;
            kill(pid, SIGSTOP);
        }
    }
    kill(pidInterCont, SIGCONT);

    pidInterCont = fork();

    if(!pidInterCont)
    {
        printf("====================Tentativa de executar o controller!============================\n");
        execle("interControllerSim2", "interControllerSim2", NULL, (char *)0);
        printf("Não foi possível executar o controller!\n");
        exit(1);
    }
    else{
        kill(pidInterCont, SIGSTOP);
    } 

    puts("Tentativa de executar fifo!");
    configuraFifos(&fifoRq, &fifoSc, &fifoOut);

    char buf [200];

    while(1)
    {
        char ch;
        int ind;

        if(pauseInt)
        {
            //fazzer o print 
            for(int i = 0; i < 5; i++)
            {
                kill(pIda[i].pid, SIGSTOP);
                printf("| %d |    %d    |    %c    | %s | %s | %d |\n", pIda->valorPC, pIda->estado, pIda->operacao, pIda->dir, pIda->subdir, pIda->isFile);

            }
            kill(pidInterCont, SIGSTOP);
            pause();
        }
        else{
            for(int i = 0; i < 5; i++)
            {
                if (pIda[i].estado == 1)
                    kill(pIda[i].pid, SIGCONT);
            }
            kill(pidInterCont, SIGCONT);
        }

        while(1)
        {
            semaforoP(semId);
            for(int i = 0; i < 5; i++)
<<<<<<< Updated upstream
            {
                if ((pIda + i)->pronto == 1)
                    ind = i;                  
            }
            semaforoV(semId);
        }

        Info* infoPronto = pIda + ind;
        Resposta* respPronto = pResp + ind;
        semaforoP(semId);
        if(infoPronto->isFile == 1) //arquivo
        {
=======
            {
                if ((pIda + i)->pronto == 1)
                {
                    ind = i;
                    break;
                }                 
            }
            semaforoV(semId);
        }

        Info* infoPronto = pIda + ind;
        Resposta* respPronto = pResp + ind;
        semaforoP(semId);

         if(infoPronto->estado == '0')
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
        if(infoPronto->isFile == 1) //arquivo
        {

>>>>>>> Stashed changes
            if(infoPronto->operacao == 'r')
            {
                int pid = excluiFila(filaD1);
                unsigned char ind = 0;
                if (pid != -1)
                {
                    ind = retornaIndPid(vPids, pid);
                    infoPronto->estado = 0;
                    vPids[ind].operacao = 0;
                    
                    printf("kernel - processo %d liberado pelo D1\n", pid);
                }
            }
            else if(infoPronto->operacao == 'w')
            {

            }
            else if(infoPronto->operacao == 'e')
            {

            }




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

        // interrupção do controller
        else if (ch == '3')
        {
            // TODO mandar os dados de print para o controler!!
            for(int i = 0; i < 5; i++)
            {
                kill(vPids[i].pid, SIGSTOP);
                char buffer[30];
                printf("Dados para a tabela do controler enviados!\n");
                int len = sprintf(buffer, "%d %d %d %c %d %d\n", vPids[i].valorPC, vPids[i].estado, vPids[i].dispositivo, vPids[i].operacao, vPids[i].qtdVzsD1, vPids[i].qtdVzsD1);
                printf(buffer);
                if (write(fifoOut, buffer, len) == -1) 
                {
                    perror("Erro ao enviar dados pela FIFO de saída da kernel!");
                    close(fifoOut);
                    close(fifoRq);
                    close(fifoSc);
                    exit(0);
                }
            }
        }



        if(read (fifoSc, buf, sizeof(buf)) > 0)
        {
            int pid, dispositivo, estado, pc;
            char operacao; 
            int index = 0;
            if(sscanf(buf, "%d %d %d %d %c\n", &pid, &pc, &dispositivo, &estado, &operacao) == 5)
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
void configuraFifos(int* fin, int* faX, int* fout)
{
    unlink(FIFO_KERNEL_IN);
    if (access(FIFO_KERNEL_IN, F_OK) == -1)
    {
        if (mkfifo (FIFO_KERNEL_IN, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO_KERNEL_IN);
            exit(-1);
        }
        puts ("FIFO KERNEL IN criada com sucesso");
    } 

    if ((*fin = open (FIFO_KERNEL_IN, ROPENMODE | O_NONBLOCK)) < 0)
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
        puts ("FIFO AX criada com sucesso");
    } 

    if ((*faX = open (FIFO_AX_IN, ROPENMODE | O_NONBLOCK)) < 0)
    {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO_AX_IN);
        exit(-2);
    }

    unlink(FIFO_KERNEL_OUT);
    if (access(FIFO_KERNEL_OUT, F_OK) == -1)
    {
        if (mkfifo (FIFO_KERNEL_OUT, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf (stderr, "Erro ao criar FIFO %s\n", FIFO_KERNEL_OUT);
            exit(-1);
        }
        puts ("FIFO KERNEL OUT criada com sucesso");
    } 

    if ((*fout = open (FIFO_KERNEL_OUT, WOPENMODE)) < 0)
    {
        fprintf (stderr, "Erro ao abrir a FIFO %s\n", FIFO_KERNEL_OUT);
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