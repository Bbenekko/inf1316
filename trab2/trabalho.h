#define PATH_MAX 100
#define PAYLOAD_MAX 16
#define NAME_MAX 50
#define ALLFILESNAME_MAX 1000

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
 