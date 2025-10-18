#include "fifo.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>

#define FIFO_SYSCALL "/tmp/fifo_syscall"
#define MAX 5

int main(int argc, char *argv[]) {
    signal(SIGINT, SIG_IGN); 

    int fd;
    srand(getpid() ^ time(NULL));

    if(openFIFO(&fd, FIFO_SYSCALL, O_WRONLY) == -1) {
        exit(1);
    }

    int PC = 0;
    printf("[App %d] - Iniciado (PID=%d)\n", getpid(), getpid());
    fflush(stdout);

    while (PC < MAX) {
        sleep(1);
        PC++;

        char dev = '-';
        char op = '-';
        char state[16] = "RUNNING";

        int r = rand() % 100;
        if (r < 15) {  // 15% chance de syscall
            dev = (rand() % 2) ? '1' : '2';
            op = "RWX"[rand() % 3];
            printf("[App %d] -> syscall(D%c, %c)\n", getpid(), dev, op);
        }

        if(PC >= MAX) {
            strcpy(state, "TERMINATED");
        }

        char msg[64];
        sprintf(msg, "%d %c %c %d %s", getpid(), dev, op, PC, state);
        write(fd, msg, strlen(msg));

        printf("[App %d] - PC=%d executando...\n", getpid(), PC);
        fflush(stdout);
    }

    printf("[App %d] - Finalizado (PC=%d)\n", getpid(), PC);
    close(fd);
    return 0;
}
