#include "stdio.h"
#include "stdlib.h"
#include "signal.h"

void capFloat(int sinal)
{
    printf("Capturou a excecao! - 0 no denominador\n");
    exit(0);
}

int main(void)
{

    //signal(SIGFPE, capFloat);

    int a, b;
    printf("Insira os numeros para as operacoes:\n");
    scanf("%d %d", &a, &b);

    printf("a + b = %d\n", a + b);
    printf("a - b = %d\n", a - b);
    printf("a * b = %d\n", a * b);
    printf("a / b = %d\n", a / b);

}