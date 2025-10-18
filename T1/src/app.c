#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define FIFO_SYSCALL "/tmp/fifo_syscall"
#define MAX 30

int main(int argc, char *argv[]) {
    int fd;
    srand(getpid() ^ time(NULL));

    // Abre FIFO para escrita
    fd = open(FIFO_SYSCALL, O_WRONLY);
    if (fd == -1) {
        perror("[App] - Erro ao abrir FIFO_SYSCALL");
        exit(1);
    }

    int PC = 0;
    printf("[App %d] - Iniciado (PID=%d)\n", getpid(), getpid());

    while (PC < MAX) {
        sleep(1);
        PC++;
        printf("[App %d] - PC=%d executando...\n", getpid(), PC);

        int r = rand() % 100;
        if (r < 15) { // 15% de chance de gerar syscall
            char dev = (rand() % 2) ? '1' : '2';
            char op = "RWX"[rand() % 3];

            char msg[32];
            sprintf(msg, "%d %c %c", getpid(), dev, op);
            write(fd, msg, strlen(msg));

            printf("[App %d] -> syscall(D%c, %c)\n", getpid(), dev, op);
        }
    }

    printf("[App %d] - Finalizado (PC=%d)\n", getpid(), PC);
    close(fd);
    return 0;
}
