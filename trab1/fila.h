typedef struct no No;

struct no {
    int pid;
    No* pProx;
    No* pAnt;
};


void insereNaFila(No* pHead);