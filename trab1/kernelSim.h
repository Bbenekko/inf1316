typedef struct info Info;

struct info
{
    int valorPC;
    unsigned char estado; // 0 - espera ; 1 - em andamento ; 2 terminado ; 3 - bloqueado
    unsigned char dispositivo; // Apenas se estiver bloqueado: 0 - nnao esta bloqueado ; 1 - D1; 2 - D2
    char operacao;
    int qtdVzsD1;
    int qtdVzsD2;
    int pid;
};

void sysCall(int dispositivo, char operacao);

