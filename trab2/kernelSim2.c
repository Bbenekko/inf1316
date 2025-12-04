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
void send_sfp_request(int sockfd, const Info *req_info);

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

    // Configura o endereço do KernelSim para escutar
    bzero((char *)&sfp_kerneladdr, sizeof(sfp_kerneladdr));
    sfp_kerneladdr.sin_family = AF_INET;
    sfp_kerneladdr.sin_addr.s_addr = htonl(INADDR_ANY);
    sfp_kerneladdr.sin_port = htons((unsigned short)SFSS_REPLY_PORT);

    if (bind(sfp_sockfd, (struct sockaddr *)&sfp_kerneladdr, sizeof(sfp_kerneladdr)) < 0)
        perror("ERROR on SFP binding");

    // Configurar o socket UDP como NÃO-BLOQUEANTE
    int flags = fcntl(sfp_sockfd, F_GETFL, 0);
    fcntl(sfp_sockfd, F_SETFL, flags | O_NONBLOCK);

    printf("KernelSim escutando SFP Replies na porta %d\n", SFSS_REPLY_PORT);

    char *nameExecs[] = {"ax1", "ax2", "ax3", "ax4", "ax5"};
    char **ptrNameExecs = nameExecs;

    pidInterCont = fork();
    if (!pidInterCont) {
        execle(ptrNameExecs[0], ptrNameExecs[0], NULL, (char *)0);
    } else {
        // O KernelSim (pai) pausa o controller imediatamente
        kill(pidInterCont, SIGSTOP); 
    }

    configuraFifos(&fifoRq);

    kill(pidInterCont, SIGCONT);

    indexPidCurrent = 0;
    pIda[0].estado = 1; 
    kill(pIda[0].pid, SIGCONT);

    pidInterCont = fork();

    if (!pidInterCont)
    {
        printf("====================Tentativa de executar o controller!============================\n");
        execle("interControllerSim2", "interControllerSim2", NULL, (char *)0);
        printf("Não foi possível executar o controller!\n");
        exit(1);
    }
    else
    {
        kill(pidInterCont, SIGSTOP);
    }

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
            // fazzer o print
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
                // ** SYSCALL PRONTA! **
                printf("KernelSim: Syscall PRONTA do PID %d (A%d)\n", (pIda + i)->pid, i + 1);

                // 1. Enviar a requisição SFP (UDP) para o SFSS
                // Você precisará de uma função que construa e envie a SFP_Packet
                send_sfp_request(sfp_sockfd, (pIda + i));

                // 2. Bloquear o processo Ax e colocá-lo na fila de espera de resposta
                kill((pIda + i)->pid, SIGSTOP);
                (pIda + i)->estado = 3; // Bloqueado
                (pIda + i)->pronto = 0; // Sinalizar que a REQ foi processada

                // Você pode usar as filas D1/D2 ou criar novas filas para REQ/REP
                // Dependendo da operação (Arquivo ou Diretório), insira o PID na fila de espera SFP
                if ((pIda + i)->isFile == 1)
                {
                    // insereFila(fileRequestWaitQueue, (pIda + i)->pid);
                }
                else
                {
                    // insereFila(dirRequestWaitQueue, (pIda + i)->pid);
                }
            }
            semaforoV(semId);
        }

        // B. PROCESSAR RESPOSTAS SFP (UDP Socket)
        SFP_Packet received_reply;

        // Tenta receber uma resposta (Não Bloqueante)
        int n_sfp = recvfrom(sfp_sockfd, (char *)&received_reply, sizeof(SFP_Packet), 0,
                             (struct sockaddr *)&sfp_serveraddr, &sfp_clientlen);

        if (n_sfp > 0)
        {
            int owner_num = received_reply.msg.owner;
            int owner_index = owner_num - 1; // 1 -> 0, 5 -> 4

            // 1. Armazenar o pacote (usando o índice 0-4)
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
            // Tratar erro real de socket, se houver
            perror("ERROR in recvfrom SFP");
        }

        // C. PROCESSAR INTERRUPÇÕES (FIFO)
        // Leitura não-bloqueante da FIFO
        if (read(fifoRq, &ch, 1) > 0)
        {
            printf("KernelSim: Recebido IRQ %c\n", ch);

            if (ch == '0') // IRQ0 - Timeslice finished
            {
                if (indexPidCurrent != -1 && vPids[indexPidCurrent].estado == 1)
                {
                    kill(vPids[indexPidCurrent].pid, SIGSTOP);
                    vPids[indexPidCurrent].estado = 0; // Coloca em estado de Pronto (Espera)
                    printf("kernel - processo %d bloqueado pelo timeslice\n", vPids[indexPidCurrent].pid);
                }

                // 2. Buscar o próximo processo pronto (estado == 0) de forma circular
                int proxId = (indexPidCurrent + 1) % 5;
                int found = 0;

                for (int i = 0; i < 5; i++)
                {
                    // O índice de busca é circular a partir do próximo (indexPidCurrent + 1)
                    if (vPids[proxId].estado == 0) // O estado 0 deve significar "Pronto para CPU"
                    {
                        // 3. Liberar e atualizar o estado
                        kill(vPids[proxId].pid, SIGCONT);

                        // **CORREÇÃO AQUI:** Use vPids[proxId].pid para o log
                        printf("kernel - processo %d liberado pelo timeslice\n", vPids[proxId].pid);

                        vPids[proxId].estado = 1; // Estado 1: Em andamento
                        indexPidCurrent = proxId;
                        found = 1;
                        break;
                    }

                    // Avança para o próximo PID (circular)
                    proxId = (proxId + 1) % 5;
                }

                if (!found)
                {
                    printf("kernel - Aviso: Nenhum processo pronto para escalonar.\n");
                }
            }
            else if (ch == '1') // IRQ1 - I/O File finished (Respostas de Arquivo)
            {
                int pid_to_unblock = excluiFila(fileReplyQueue);
                if (pid_to_unblock != -1)
                {
                    unsigned char ind = retornaIndPid(vPids, pid_to_unblock); // Obtém o índice 0-4

                    // 1. TRANSFERIR A RESPOSTA (do reply_vec[ind]) para a SHM (pResp[ind])
                    semaforoP(semId);

                    // Copiar os dados relevantes
                    memcpy(pResp[ind].dados, reply_vec[ind].msg.payload, PAYLOAD_MAX);
                    pResp[ind].valorRetorno = reply_vec[ind].msg.offset; // offset negativo se erro
                    pResp[ind].pronto = 1;

                    pIda[ind].estado = 0; // Desbloqueado, pronto para rodar
                    kill(pid_to_unblock, SIGCONT);

                    semaforoV(semId);
                }
            }
            else if (ch == '2') // IRQ2 - I/O Directory finished (Respostas de Diretório)
            {
                int pid_to_unblock = excluiFila(dirReplyQueue);
                if (pid_to_unblock != -1)
                {
                    unsigned char ind = retornaIndPid(vPids, pid_to_unblock);

                    // 1. TRANSFERIR A RESPOSTA (do reply_vec)
                    semaforoP(semId);

                    // Para Diretório, o payload é usado para o nome e valorRetorno é o strlen ou código de erro.
                    // Os dados necessários dependem do DC_REP, DR_REP ou DL_REP.
                    // Para DC/DR, a resposta está em msg.pathName. Você deve ajustar a struct Resposta.
                    // Assumindo que você usa pResp[ind].dados para a string de path ou dados DL_REP.

                    // Exemplo para DC/DR REP:
                    // memcpy(pResp[ind].dados, reply_vec[ind].msg.pathName, PATH_MAX);
                    pResp[ind].valorRetorno = reply_vec[ind].msg.offset; // Ou sizePathName, dependendo do REP
                    pResp[ind].pronto = 1;

                    // 2. Desbloquear o processo Ax
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
        // vInfos[i].dir;
        vInfos[i].estado = 0;
        vInfos[i].isFile = 0;
        vInfos[i].modo = 0;
        vInfos[i].offset = 0;
        vInfos[i].pronto = 0;
        // vInfos[i].subdir;
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

void send_sfp_request(int sockfd, const Info *req_info)
{
    struct sockaddr_in serveraddr;
    struct hostent *server;
    SFP_Packet request;
    int n;

    // 1. Construir o SFP_Packet a partir da Info

    // Mapear operação para tipo SFP
    if (req_info->operacao == 'r')
        request.type = SFP_RD_REQ;
    else if (req_info->operacao == 'w')
        request.type = SFP_WR_REQ;
    else if (req_info->operacao == 'a')
        request.type = SFP_DC_REQ; // 'a' para add/create
    else if (req_info->operacao == 'e')
        request.type = SFP_DR_REQ; // 'e' para exclude/remove
    else if (req_info->operacao == 'l')
        request.type = SFP_DL_REQ; // 'l' para listdir
    else
    {
        fprintf(stderr, "ERRO: Tipo de operação SFP desconhecido: %c\n", req_info->operacao);
        return;
    }

    // Preencher a struct Message
    request.msg.owner = req_info->dir[1] - '0'; // Ex: 'A1' -> 1
    request.msg.offset = req_info->offset;

    // Construir o path completo (dir/subdir) para o SFP
    char full_path[PATH_MAX];
    snprintf(full_path, PATH_MAX, "%s%s", req_info->dir, req_info->subdir);

    strncpy(request.msg.pathName, full_path, PATH_MAX - 1);
    request.msg.pathName[PATH_MAX - 1] = '\0';
    request.msg.sizePathName = (int)strlen(request.msg.pathName);

    // Para DC (add), o dirName é o subdiretório. Para WR, o payload é o que deve ser escrito.
    // **NOTA:** Aqui você precisaria adaptar a Info para saber qual é o payload/dirname.
    // Assumindo que o payload e dirname estão na Info (adaptar se necessário).
    // strcpy(request.msg.payload, req_info->payload);
    // strcpy(request.msg.dirName, req_info->dirName);

    // 2. Configurar o endereço do servidor SFSS (RECEPTOR)
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
    serveraddr.sin_port = htons(SFSS_SERVER_PORT); // Porta do Servidor

    // 3. Enviar o pacote SFP
    int serverlen = sizeof(serveraddr);
    n = sendto(sockfd, (char *)&request, sizeof(SFP_Packet), 0, (struct sockaddr *)&serveraddr, serverlen);

    if (n < 0)
        perror("ERROR in sendto SFP REQ");
    else
        printf("KernelSim: Enviado SFP REQ Tipo %d (Owner %d) para SFSS.\n", request.type, request.msg.owner);
}