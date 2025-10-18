#include "fila.h"
#include <stdio.h>

struct no {
    int pid;
    No* pProx;
    No* pAnt;
};

Fila* criaFila(void) {
	Fila* fila = (Fila*)malloc(sizeof(Fila));

	if (!fila) {
		exit(2);
	}

	fila->pIni = fila->pFin = NULL;	// Inicializa ponteiros com NULL

	return fila;
}

No* inserePilha(No* pilha, int pid) {
	No* elemento = (No*)malloc(sizeof(No));	// Aloca dinamicamente novo n�
	if (!elemento) {
		exit(2);
	}
	elemento->pid = pid;					// Preenche a estrutura com o n�mero fornecido
	elemento->pProx = pilha;				// Interliga � pilha existente
	return elemento;						// Retorna o novo topo da pilha
}

void insereFila(Fila* fila, int pid) {
	No* no = inserePilha(NULL, pid); // A fun��o inserePilha serve igualmente para a cria��o de estruturas de tipo No quando pProx = NULL

	if (!(fila->pIni)) {				// Se pIni == NULL, a fila est� vazia
		fila->pIni = fila->pFin = no;	// Logo, os dois ponteiros auxiliares dever�o apontar para o n� a ser adicionado
	}

	else if (!(fila->pIni->pProx)) {	// Se pIni->pProx == NULL, a fila cont�m apenas 1 elemento
		fila->pIni->pProx = no;			// Logo, o n� existente dever� apontar para o novo n�
		fila->pFin = no;				// E pFin dever� apontar para o novo n�, que comp�e o fim da fila
	}

	else {								// A fila possui dois ou mais elementos
		fila->pFin->pProx = no;			// Logo, o n� do fim deve apontar para o novo n� a ser inserido
		fila->pFin = no;				// E pFin dever� apontar para o novo n�, que comp�e o novo fim da fila
	}
}

void excluiFila(Fila* fila) {

	if (!(fila->pIni)) {	// Se pIni == NULL, a fila est� vazia. Logo, n�o h� remo��es a fazer
		printf("\n\nAviso: Fila vazia.\n\n");
		return;
	}

	No* carregador = fila->pIni;	// Guarda endere�o do n� a ser removido do in�cio
	fila->pIni = fila->pIni->pProx;	// Define novo primeiro da fila

	free(carregador);				// Libera mem�ria do removido
}

void imprimeFila(Fila* fila) {
	No* pAtual = fila->pIni;
	int qtd = 0;
	while (pAtual) { // Loop at� alcan�ar fim da fila, isto �, pAtual == NULL, que equivale a false
		printf("[%d]: %d ", qtd, pAtual->pid);	// Imprime conte�do do n� atual
		qtd++;									// Atualiza contador de �ndices
		pAtual = pAtual->pProx;					// Atualiza n�
	}

	printf("\n");
}
