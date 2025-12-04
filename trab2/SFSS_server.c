#include "trabalho.h"

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char **argv)
{
    int sockfd;                    /* socket */
    int portno;                    /* port to listen on */
    int clientlen;                 /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    struct hostent *hostp;         /* client host info */
    char buf[BUFSIZE];             /* message buf */
    char *hostaddrp;               /* dotted decimal host addr string */
    int optval;                    /* flag value for setsockopt */
    int n;                         /* message byte size */

    /*
     * check command line arguments
     */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    portno = atoi(argv[1]);

    /*
     * socket: create the parent socket
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* setsockopt: Handy debugging trick that lets
     * us rerun the server immediately after we kill it;
     * otherwise we have to wait about 20 secs.
     * Eliminates "ERROR on binding: Address already in use" error.
     */
    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval, sizeof(int));

    /*
     * build the server's Internet address
     */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    /*
     * bind: associate the parent socket with a port
     */
    if (bind(sockfd, (struct sockaddr *)&serveraddr,
             sizeof(serveraddr)) < 0)
        error("ERROR on binding");

    /*
     * main loop: wait for a datagram, then echo it
     */
    clientlen = sizeof(clientaddr);
    while (1)
    {

        // Adaptação no main do udpserver.c, dentro do loop while (1):

        clientlen = sizeof(clientaddr);

        /* recvfrom: receive a UDP datagram (the struct) */
        SFP_Packet request;
        n = recvfrom(sockfd, (char *)&request, sizeof(SFP_Packet), 0,
                     (struct sockaddr *)&clientaddr, &clientlen);
        if (n < 0)
            error("ERROR in recvfrom");

        printf("Server received %d bytes. Type: %d\n", n, request.type);

        // 1. Processar a requisição e gerar a resposta
        SFP_Packet reply;
        reply.type = request.type + 1;       // REQ -> REP
        reply.msg.owner = request.msg.owner; // Ecoar o owner [cite: 106]

        switch (request.type)
        {
        case SFP_WR_REQ:
            // Chamar sua função de escrita:
            // write_file(&request.msg, &reply.msg);
            break;
        case SFP_DC_REQ:
            // Chamar sua função de criação de diretório:
            // create_subDirectory(&request.msg, &reply.msg);
            break;
        // ... outros casos
        default:
            // Tratar erro de tipo
            break;
        }

        /* sendto: echo the reply struct back to the client */
        n = sendto(sockfd, (char *)&reply, sizeof(SFP_Packet), 0,
                   (struct sockaddr *)&clientaddr, clientlen);
        if (n < 0)
            error("ERROR in sendto");
    }
}
