#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/sem.h>

#include "fila.h"
#include "semaforo.h"
#include "trabalho.h"

// mensagens do controller para o kernel
#ifndef FIFO_KERNEL_IN
#define FIFO_KERNEL_IN "kernelFifoInt"
#endif

#define ROPENMODE (O_RDONLY)

#define SFSS_REPLY_PORT 12346
#define SFSS_SERVER_PORT 12345
#define SFSS_HOSTNAME "localhost"

// vetor com Id dos Pids
Info vPids[5];

// filas de processos
Fila *filaD1;
Fila *filaD2;
Fila *fileReplyQueue;
Fila *dirReplyQueue;

// arquivos FIFO
int fifoRq;

int pidInterCont = 0;

// pid do processo atual
int indexPidCurrent = -1;
int pauseInt = 0;

unsigned char retornaIndPid(Info vPids[], int pid);
void iniciaVetor(Info vInfos[]);
void configuraFifos(int *fin);
void stopHandler(int num);
void mandar_requisicao(int sockfd, const Info *req_info);

SFP_Packet reply_vec[5];

int main()
{

    signal(SIGINT, stopHandler);
    iniciaVetor(vPids);

    filaD1 = criaFila();
    filaD2 = criaFila();

    fileReplyQueue = criaFila();
    dirReplyQueue = criaFila();

    __key_t chaveMemVinda = 8751;
    __key_t chaveMemResp = 8752;
    __key_t chaveSem = 87511;

    int segmento = shmget(chaveMemVinda, sizeof(Info) * 5, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (segmento < 0)
    {
        perror("shmget");
        exit(1);
    }
    Info *pIda = (Info *)shmat(segmento, 0, 0);

    int segmento2 = shmget(chaveMemResp, sizeof(Resposta) * 5, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (segmento2 < 0)
    {
        perror("shmget");
        exit(1);
    }
    Resposta *pResp = (Resposta *)shmat(segmento2, 0, 0);

    int semId = semget(chaveSem, 1, 0666 | IPC_CREAT);
    printf("SemID: %d\n", semId);

    configuraFifos(&fifoRq);

    int sfp_sockfd;
    struct sockaddr_in sfp_serveraddr; // Para guardar o endereço do servidor SFSS (REPLY sender)
    struct sockaddr_in sfp_kerneladdr; // Endereço do KernelSim
    int sfp_clientlen = sizeof(sfp_serveraddr);

    sfp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfp_sockfd < 0)
        perror("ERROR opening SFP socket");


    bzero((char *)&sfp_kerneladdr, sizeof(sfp_kerneladdr));
    sfp_kerneladdr.sin_family = AF_INET;
    sfp_kerneladdr.sin_addr.s_addr = htonl(INADDR_ANY);
    sfp_kerneladdr.sin_port = htons((unsigned short)SFSS_REPLY_PORT);

    if (bind(sfp_sockfd, (struct sockaddr *)&sfp_kerneladdr, sizeof(sfp_kerneladdr)) < 0)
        perror("ERROR on SFP binding");


    int flags = fcntl(sfp_sockfd, F_GETFL, 0);
    fcntl(sfp_sockfd, F_SETFL, flags | O_NONBLOCK);

    printf("KernelSim escutando SFP Replies na porta %d\n", SFSS_REPLY_PORT);

    configuraFifos(&fifoRq);
    puts("FIFO KERNEL IN criada e aberta para leitura.");

    configuraFifos(&fifoRq);
    puts("FIFO KERNEL IN criada e aberta para leitura.");

    // b) Lançamento do InterControllerSim2 (Controller)
    pidInterCont = fork();

    if (!pidInterCont) // Filho (Controller)
    {
         execle("interControllerSim2", "interControllerSim2", NULL, (char *)0);
    }
    else // Pai (KernelSim)
    {
        kill(pidInterCont, SIGSTOP); // Controller parado
    }

    char *nameExecs[] = {"ax1", "ax2", "ax3", "ax4", "ax5"};

    for (int i = 0; i < 5; i++)
    {
        int pid = fork();
        if (pid == 0) // filho
        {
            printf("Executando filho A%d!\n", i + 1);
            execle(nameExecs[i], nameExecs[i], NULL, (char *)0);
            exit(1); 
        }
        else 
        {
            vPids[i].pid = pid;
            pIda[i].pid = pid;  
            pIda[i].estado = 0; 
            kill(pid, SIGSTOP);
        }

        setSemValue(semId);

        kill(pidInterCont, SIGCONT);
        printf("KernelSim: Controller liberado para iniciar IRQs.\n");

        indexPidCurrent = 0;
        pIda[0].estado = 1; // Em andamento
        kill(pIda[0].pid, SIGCONT);
        printf("KernelSim: Início da simulação. Processo A1 em execução.\n");
    }

    kill(pidInterCont, SIGCONT);

    indexPidCurrent = 0;
    pIda[0].estado = 1;
    kill(pIda[0].pid, SIGCONT);

    puts("Tentativa de executar fifo!");
    configuraFifos(&fifoRq);

    kill(pidInterCont, SIGCONT);

    char buf[200];

    while (1)
    {
        char ch;
        int ind;

        if (pauseInt)
        {
            // fazer o print
            for (int i = 0; i < 5; i++)
            {
                kill(pIda[i].pid, SIGSTOP);
                printf("| %d |    %d    |    %c    | %s | %s | %d |\n", pIda->valorPC, pIda->estado, pIda->operacao, pIda->dir, pIda->subdir, pIda->isFile);
            }
            kill(pidInterCont, SIGSTOP);
            pause();
        }
        else
        {
            for (int i = 0; i < 5; i++)
            {
                if (pIda[i].estado == 1)
                    kill(pIda[i].pid, SIGCONT);
            }
            kill(pidInterCont, SIGCONT);
        }

        for (int i = 0; i < 5; i++)
        {
            semaforoP(semId);
            if ((pIda + i)->pronto == 1)
            {
                printf("KernelSim: Syscall PRONTA do PID %d (A%d)\n", (pIda + i)->pid, i + 1);

                mandar_requisicao(sfp_sockfd, (pIda + i));

                kill((pIda + i)->pid, SIGSTOP);
                (pIda + i)->estado = 3; 
                (pIda + i)->pronto = 0; 

                if ((pIda + i)->isFile == 1)
                {
                     insereFila(fileReplyQueue, (pIda + i)->pid);
                }
                else
                {
                     insereFila(dirReplyQueue, (pIda + i)->pid);
                }
            }
            semaforoV(semId);
        }

        SFP_Packet received_reply;

        int n_sfp = recvfrom(sfp_sockfd, (char *)&received_reply, sizeof(SFP_Packet), 0,
                             (struct sockaddr *)&sfp_serveraddr, &sfp_clientlen);

        if (n_sfp > 0)
        {
            int owner_num = received_reply.msg.owner;
            int owner_index = owner_num - 1; 

            memcpy(&reply_vec[owner_index], &received_reply, sizeof(SFP_Packet));

            int pid_to_queue = pIda[owner_index].pid;

            if (received_reply.type == SFP_RD_REP || received_reply.type == SFP_WR_REP)
            {
                insereFila(fileReplyQueue, pIda[owner_index].pid);
            }
            else
            {
                insereFila(dirReplyQueue, pIda[owner_index].pid);
            }
        }
        else if (n_sfp < 0 && errno != EWOULDBLOCK)
        {
            
            perror("ERROR em recvfrom SFP");
        }

        if (read(fifoRq, &ch, 1) > 0)
        {
            printf("KernelSim: Recebido IRQ %c\n", ch);

            if (ch == '0')
            {
                if (indexPidCurrent != -1 && vPids[indexPidCurrent].estado == 1)
                {
                    kill(vPids[indexPidCurrent].pid, SIGSTOP);
                    vPids[indexPidCurrent].estado = 0; 
                    printf("kernel - processo %d bloqueado pelo timeslice\n", vPids[indexPidCurrent].pid);
                }

                int proxId = (indexPidCurrent + 1) % 5;
                int found = 0;

                for (int i = 0; i < 5; i++)
                {
                    if (vPids[proxId].estado == 0) 
                    {
                        kill(vPids[proxId].pid, SIGCONT);

                        printf("kernel - processo %d liberado pelo timeslice\n", vPids[proxId].pid);

                        vPids[proxId].estado = 1;
                        indexPidCurrent = proxId;
                        found = 1;
                        break;
                    }

                    proxId = (proxId + 1) % 5;
                }

                if (!found)
                {
                    printf("kernel - Aviso: Nenhum processo pronto para escalonar.\n");
                }
            }
            else if (ch == '1')
            {
                int pid_to_unblock = excluiFila(fileReplyQueue);
                if (pid_to_unblock != -1)
                {
                    unsigned char ind = retornaIndPid(vPids, pid_to_unblock); 

                    semaforoP(semId);

                    memcpy(pResp[ind].dados, reply_vec[ind].msg.payload, PAYLOAD_MAX);
                    pResp[ind].valorRetorno = reply_vec[ind].msg.offset; 
                    pResp[ind].pronto = 1;

                    pIda[ind].estado = 0; 
                    kill(pid_to_unblock, SIGCONT);

                    semaforoV(semId);
                }
            }
            else if (ch == '2') 
            {
                int pid_to_unblock = excluiFila(dirReplyQueue);
                if (pid_to_unblock != -1)
                {
                    unsigned char ind = retornaIndPid(vPids, pid_to_unblock);

                    semaforoP(semId);

                    pResp[ind].valorRetorno = reply_vec[ind].msg.offset; 
                    pResp[ind].pronto = 1;

                    pIda[ind].estado = 0;
                    kill(pid_to_unblock, SIGCONT);

                    semaforoV(semId);
                    printf("kernel - processo %d (IRQ2) liberado com resposta\n", pid_to_unblock);
                }
            }
        }
    }

    return 0;
}

// realiza a abertura das fifos
void configuraFifos(int *fin)
{
    unlink(FIFO_KERNEL_IN);
    if (access(FIFO_KERNEL_IN, F_OK) == -1)
    {
        if (mkfifo(FIFO_KERNEL_IN, S_IRUSR | S_IWUSR) != 0)
        {
            fprintf(stderr, "Erro ao criar FIFO %s\n", FIFO_KERNEL_IN);
            exit(-1);
        }
        puts("FIFO KERNEL IN criada com sucesso");
    }

    if ((*fin = open(FIFO_KERNEL_IN, ROPENMODE | O_NONBLOCK)) < 0)
    {
        fprintf(stderr, "Erro ao abrir a FIFO %s\n", FIFO_KERNEL_IN);
        exit(-2);
    }
}

void iniciaVetor(Info vInfos[])
{
    for (int i = 0; i < 5; i++)
    {
        vInfos[i].estado = 0;
        vInfos[i].isFile = 0;
        vInfos[i].modo = 0;
        vInfos[i].offset = 0;
        vInfos[i].pronto = 0;
        vInfos[i].valorPC = 0;
        vInfos[i].pid = 0;
        vInfos[i].valorPC = 0;
        vInfos[i].operacao = '\0';
    }
}

unsigned char retornaIndPid(Info vPids[], int pid)
{
    int j = 0;
    for (; j < 5; j++)
    {
        if (vPids[j].pid == pid)
            break;
    }

    return j;
}

void stopHandler(int num)
{
    pauseInt = !pauseInt;
}

void mandar_requisicao(int sockfd, const Info *req_info)
{
    struct sockaddr_in serveraddr;
    struct hostent *server;
    SFP_Packet request;
    int n;

    if (req_info->operacao == 'r')
        request.type = SFP_RD_REQ;
    else if (req_info->operacao == 'w')
        request.type = SFP_WR_REQ;
    else if (req_info->operacao == 'a')
        request.type = SFP_DC_REQ; // 'a' para add
    else if (req_info->operacao == 'e')
        request.type = SFP_DR_REQ; // 'e' para exclude
    else if (req_info->operacao == 'l')
        request.type = SFP_DL_REQ; // 'l' para list
    else
    {
        fprintf(stderr, "ERRO: Tipo de operação SFP desconhecido: %c\n", req_info->operacao);
        return;
    }

    
    request.msg.owner = req_info->dir[1] - '0'; // Ex: 'A1' -> 1
    request.msg.offset = req_info->offset;

    char full_path[PATH_MAX];
    snprintf(full_path, PATH_MAX, "%s%s", req_info->dir, req_info->subdir);

    strncpy(request.msg.pathName, full_path, PATH_MAX - 1);
    request.msg.pathName[PATH_MAX - 1] = '\0';
    request.msg.sizePathName = (int)strlen(request.msg.pathName);

    server = gethostbyname(SFSS_HOSTNAME);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host as %s\n", SFSS_HOSTNAME);
        return;
    }

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(SFSS_SERVER_PORT); 

    int serverlen = sizeof(serveraddr);
    n = sendto(sockfd, (char *)&request, sizeof(SFP_Packet), 0, (struct sockaddr *)&serveraddr, serverlen);

    if (n < 0)
        perror("ERROR in sendto SFP REQ");
    else
        printf("KernelSim: Enviado SFP REQ Tipo %d (Owner %d) para SFSS.\n", request.type, request.msg.owner);
}