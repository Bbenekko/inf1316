typedef struct info Info;

struct info
{
    int valorPC;
    unsigned char estado; // 0 - parado ; 1 - em andamento
    unsigned char dispositivo; // Apenas se estiver bloqueado: 0 - nnao esta bloqueado ; 1 - D1; 2 - D2
    char operacao;
    int qtdVezesParado;
    unsigned char estaTerminado; // 0 - nao ; 1 - sim
    int qtdVzsD1;
    int qtdVzsD2;
};

void sysCall(int dispositivo, char operacao);