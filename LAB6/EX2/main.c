#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define FIFO "aiChavinho"

int main(void)
{
    if(access(FIFO, F_OK) == -1)
    {
        if(mkfifo(FIFO, S_IRUSR | S_IWUSR) == -1)
        {  
            printf("Problema na criacao FIFO");
            return -1;
        }
    }
    


    char *msg[] = {"M1: Sistemas Operacionais ", "M2: Let's GOOOOO\n"};
    int pid;

    int fifoPai = open(FIFO, O_RDONLY | O_NONBLOCK);
    if(fifoPai < 0)
    {
        printf("Problema na abertura FIFO pai\n");
        return -1;
    }


    for(int i = 0; i < 2; i++)
    {
        pid = fork();
        if(pid == 0)
        {
            int fifo = open(FIFO, O_WRONLY);
            if(fifo < 0)
            {
                printf("Problema na abertura FIFO\n");
                return -2;
            }         
            printf("Filho %d vai escrever: M%d\n", i+1,i+1);   
            write(fifo, msg[i], strlen(msg[i]));
            close(fifo);  
            return 0;     
        }
    }

    for(int i = 0; i < 2; i++)
        wait(NULL);


    char buffer[100];
    int n;
    while((n = read(fifoPai, buffer, sizeof(buffer)-1)) > 0)
    {
        buffer[n] = '\0';
        printf("Pai leu: %s", buffer);
    }

    close(fifoPai);
    return 0;
}
