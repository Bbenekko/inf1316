#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

void exibe(int v[])
{
    printf("Vetor: {");
    for(int i = 0; i < 10; i++)
    {
        printf("%d, ", v[i]);
    }
    printf("}\n");
}

void quicksort(int num, int *vetor) {

if (num <= 1)

return;

else {

int x = vetor[0];

int a = 1;

int b = num - 1;

do {

while (a < num && vetor[a] <= x)

a++;

while (vetor[b] > x)

b--;

if (a < b) {

int aux = vetor[a];

vetor[a] = vetor[b];

vetor[b] = aux;

a++;

b--;

}

} while (a <= b);

vetor[0] = vetor[b];

vetor[b] = x;

quicksort(b, vetor);

quicksort(num - a, &vetor[a]);

}
}

int main(void)
{
    int  pid, status;
    int vec[] = {7, 8, 9, 0, 4, 5, 6, 1 ,2 ,3};
    exibe(vec);
    pid = fork();
    if (pid!=0)
    { //Pai
        printf("Pai, Pid proprio: %d e Pid do filho: %d\n", getpid(), pid);
        waitpid(-1, &status, 0);
        exibe(vec);
    }else 
    { //Filho
        printf("Filho,  Pid: %d\n", getpid());
        quicksort(10, vec);
        exibe(vec);
        printf("Programa terminado!\n");
        exit(3);
    }
    return 0;
}