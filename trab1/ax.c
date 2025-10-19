#include <signal.h>
#include <stdio.h>

#define D1 1
#define D2 0

#define R 0
#define W 1
#define X 2

void stopHandler(int signal);
void contHandler(int signal);

int PC = 0;
int MAX = 40;
int Dx;
int Op;

int main(void)
{
    while (PC < MAX) {
        sleep(0.5);
        int d = rand()%100 +1;
        if (d  < 15) { // generate a random syscall
            if (d % 2) Dx = D1;
            else Dx= D2;
            if (d % 3 == 1) Op = R;
            else if (d % 3 == 1) Op = W;
            else Op = X;
            syscall (Dx, Op);
        }
        sleep(0.5);
    }
}

void stopHandler(int signal)
{

    printf("%d", PC);
}

void contHandler(int signal)
{

}