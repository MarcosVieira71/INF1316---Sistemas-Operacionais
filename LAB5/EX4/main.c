#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pipe(fd);
    pid_t r1, r2, writer;

    writer = fork();
    if (writer == 0) {
        close(fd[0]);
        char *mensagens[] = {"Sistemas Operacionais", "é uma", "disciplina", "divertida"};
        for (int i = 0; i < 4; i++) {
            write(fd[1], mensagens[i], strlen(mensagens[i]) + 1);
            printf("Writer: %s\n", mensagens[i]);
            sleep(1);
        }
        close(fd[1]);
        exit(0);
    }

    r1 = fork();
    if (r1 == 0) {
        close(fd[1]);
        char buffer[128];
        while (read(fd[0], buffer, sizeof(buffer)) > 0) {
            printf("Reader 1: %s\n", buffer);
            sleep(2);
        }
        close(fd[0]);
        exit(0);
    }

    r2 = fork();
    if (r2 == 0) {
        close(fd[1]);
        char buffer[128];
        while (read(fd[0], buffer, sizeof(buffer)) > 0) {
            printf("Reader 2: %s\n", buffer);
            sleep(2); 
        }
        close(fd[0]);
        exit(0);
    }



    close(fd[0]);
    close(fd[1]);

    waitpid(r1, NULL, 0);
    waitpid(r2, NULL, 0);
    waitpid(writer, NULL, 0);

    return 0;
}
