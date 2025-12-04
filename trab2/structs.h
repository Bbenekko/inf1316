#define PATH_MAX 100
<<<<<<< HEAD
#define PAYLOAD_MAX 16
=======
#define PAYLOAD_MAX 17 // inclui 0 /0
>>>>>>> refs/heads/bento
#define NAME_MAX 50
#define ALLFILESNAME_MAX 1000

#define ROOT_directory "./SFSS-root-dir"

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
 