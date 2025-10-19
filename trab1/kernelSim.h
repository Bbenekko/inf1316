typedef struct info Info;

struct info
{
    int valorPC = -1;
    unsigned char estado = -1; // 0 - parado ; 1 - em andamento
    unsigned char dispositivo = -1; // Apenas se estiver bloqueado: 0 - nnao esta bloqueado ; 1 - D1; 2 - D2
    char operacao = '\0';
    int qtdVezesParado = -1;
    unsigned char estaTerminado = -1; // 0 - nao ; 1 - sim
};