#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define OPENMODE (O_WRONLY)
#define FIFO "FIFO1"

int main(void)
{
    if(access(FIFO, F_OK) == -1)
    {
        if(mkfifo(FIFO, S_IRUSR | S_IWUSR) == -1)
        {  
            printf("Problema na criacao FIFO\n");
            return -1;
        }
    }

    int fifo;
    if((fifo = open(FIFO, OPENMODE)) < 0)
    {
        printf("Problema na abertura FIFO\n");
        return -2;      
    }

    char buffer[100];
    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (buffer[0] == ';') break;
        write(fifo, buffer, strlen(buffer));
    }

    close(fifo);

}