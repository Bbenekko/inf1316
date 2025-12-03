#define PATH_MAX 100
#define PAYLOAD_MAX 17 // inclui 0 /0
#define NAME_MAX 50
#define ALLFILESNAME_MAX 1000

typedef struct 
{
    int posInicial;
    int posFinal;
    int ehSubdiretorio;
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
 