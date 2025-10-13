#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define OPENMODE (O_RDONLY)
#define FIFO "FIFO1"

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

    int fifo;
    if((fifo = open(FIFO, OPENMODE)) < 0)
    {
        printf("Problema na abertura FIFO\n");
        return -2;          
    }
    char buf[100];
    int n;
    while ((n = read(fifo, buf, sizeof(buf)-1)) > 0) {
        buf[n] = '\0';
        printf("Printou: %s", buf);
        fflush(stdout);
    }
    close(fifo);

}