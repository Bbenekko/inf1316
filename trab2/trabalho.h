#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <strings.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PATH_MAX 100
#define PAYLOAD_MAX 17 // inclui 0 /0
#define NAME_MAX 50
#define ALLFILESNAME_MAX 1000

#define BUFSIZE 1024

#define ROOT_DIRECTORY "./SFSS-root-dir"

typedef struct 
{
    int posInicial;
    int posFinal;
    int ehSubdiretorio; // ou 0 (arquivo) ou 1 (diretório)
} fstlpositions;

typedef struct
{
    int owner;
    char pathName[PATH_MAX];
    int sizePathName;
    char payload[PAYLOAD_MAX];
    int offset;
    char dirName[NAME_MAX];
    int sizeDirName;
} Message;

typedef struct 
{
    int owner;
    char allfilesnames[ALLFILESNAME_MAX];
    fstlpositions posicoes[40];
    int nrnames;
} DL_REP;
 
/*
=============================================================
EXTRAS
=============================================================
*/

typedef enum {
    SFP_RD_REQ = 1,
    SFP_WR_REQ,
    SFP_DC_REQ,
    SFP_DR_REQ,
    SFP_DL_REQ,
    
    SFP_RD_REP, // RESPOSTAS
    SFP_WR_REP,
    SFP_DC_REP,
    SFP_DR_REP,
    SFP_DL_REP
} SFP_TYPE;

typedef struct {
    SFP_TYPE type; 
    Message msg;   
    DL_REP dl_rep;
} SFP_Packet;

typedef struct info Info;
typedef struct resposta Resposta;

struct info
{
    int valorPC;
    unsigned char estado; // 0 - espera ; 1 - em andamento ; 2 terminado ; 3 - bloqueado
    unsigned char modo; // Apenas se estiver bloqueado: 0 - nnao esta bloqueado ; 1 - leitura; 2 - escrita
    int isFile; // Apenas se estiver bloqueado: 0 - diretorio ; 1 - arquivo
    char operacao;
    char dir[3];
    char subdir[100];
    int offset;
    int pid;
    int pronto; // 0 - nao ; 1 - sim
};

struct resposta
{
    int valorRetorno;
    char dados[PAYLOAD_MAX];
    int pronto; // 0 - nao ; 1 - sim
};


/*
=============================================================
CABECÁRIOS DAS FUNÇÕES DE SFSS_functions.c
=============================================================
*/

void verifyPath(char path[], int sizePath, int owner);

void read_directory(Message* msg, Message* response);

void putNameInResponse(DL_REP* response, const char* name, int isDir);

void dl(Message* msg, DL_REP* response);

void list_directory(Message* msg, DL_REP* response);

void removeRecursivo(Message* msg, Message* response);

void exclude_diretory(Message* msg, Message* response);

void create_subDirectory(Message* dc_req, Message* dc_rep);

void write_file(Message* wr_req, Message* wr_rep);
