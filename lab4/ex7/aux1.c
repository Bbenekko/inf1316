#include <stdio.h>
#include <unistd.h>

int main(void)
{
    while(1)
    {
        printf("Filho 1 aqui ");
        usleep(10000);
    }        
}