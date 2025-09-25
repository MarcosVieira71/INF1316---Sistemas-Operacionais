#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int fd[2];
    pipe(fd);

    if (fork() == 0)
    { 
        close(fd[0]);
        char texto[100];   
        FILE* f = fopen("texto.txt", "r");
        fgets(texto, 100, f);
        write(fd[1], texto, 100);
        fclose(f);
    }
    else
    {
        waitpid(-1, NULL, 0);
        close(fd[1]);
        char texto[100];   
        read(fd[0], texto, 100);
        FILE* f = fopen("texto2.txt", "w");
        fprintf(f, "%s", texto);    
        fclose(f);    
    }
    return 0;
}