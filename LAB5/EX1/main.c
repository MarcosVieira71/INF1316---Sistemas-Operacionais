#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int fd[2];
    pipe(fd);

    char texto[1024];
    if (fork() == 0)
    { 
        close(fd[0]);
        write(fd[1], texto, strlen(texto) + 1);
    }
    else
    {
        waitpid(-1, NULL, 0);
        close(fd[1]);
        char t[strlen(texto)+1];
        read(fd[0], t, strlen(texto) + 1);
        printf("Texto lido: %s\n", t);
    }

    
}