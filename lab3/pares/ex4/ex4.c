    #include <stdio.h>
    #include <signal.h>
    #include <stdlib.h>
    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <sys/wait.h>

    #define EVER ;;

    int main()
    {
        int pid1, pid2;

        printf("pid do pai = %d\n", getpid());  
        if((pid1 = fork()) == 0) // filho 1
        {
            for(EVER)
            {
                printf("Filho 1 executando!\n");
                sleep(1);
            }
        }

        printf("pid do pai = %d\n", getpid());
        if((pid2 = fork()) == 0) // filho 2
        {
            for(EVER)
            {
                printf("Filho 2 executando!\n");
                sleep(1);
            }
        }

        for (int i = 1; i < 11; i++)
        {
            if(i%2) // se i for impar 
            {
                kill(pid1, SIGSTOP);
                kill(pid2, SIGCONT);
            }
            else // se i for par
            {
                kill(pid2, SIGSTOP);
                kill(pid1, SIGCONT);
            }
            sleep(2);
        }

        printf("Terminou!");
        kill(pid1, SIGKILL);
        kill(pid2, SIGKILL);


        return 0;
    }