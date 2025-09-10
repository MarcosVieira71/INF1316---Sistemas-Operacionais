#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void foo()
{
    while(1){
        printf("Filho %d fazendo algo\n", getpid());
        sleep(1);
    }
}

int main(void){
    int numTrades = 10;
    pid_t c1 = fork();
    if(c1 == 0)
    {
        foo();
    }
    pid_t c2 = fork();
    if(c2 == 0) 
    {
        foo();
    }
    
    else
    {
        kill(c1, SIGSTOP);
        kill(c2, SIGSTOP);
        for(int i = 0; i < numTrades; i++)
        {
            if(i % 2 == 0)
            {
                kill(c1, SIGCONT);
                kill(c2, SIGSTOP);
            }
            else
            {
                kill(c1, SIGSTOP);
                kill(c2, SIGCONT);
            }
            sleep(1);
        }
    
        kill(c1, SIGKILL);
        kill(c2, SIGKILL);
           
    }

    return 0;
}