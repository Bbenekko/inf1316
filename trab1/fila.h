typedef struct no No;

typedef struct fila Fila;	

struct fila {
	struct no* pIni;		// Ponteiro para o primeiro n� da fila
	struct no* pFin;		// Ponteiro para o �ltimo n� da fila
};

Fila* criaFila(void);

void insereFila(Fila* fila, int pid);

void excluiFila(Fila* fila);

void imprimeFila(Fila* fila);
