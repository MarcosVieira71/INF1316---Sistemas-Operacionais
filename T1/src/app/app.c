#include "fifo.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX 20

int main(int argc, char *argv[]) {

    int fd;

    srand(getpid() ^ time(NULL));

    if(openFIFO(&fd, FIFO_SYSCALL, O_WRONLY) == -1) {
        exit(1);
    }

    int PC = 0;
    int pid = getpid();
    printf("[App %d] - Iniciado (PID=%d)\n", pid, pid); 
    fflush(stdout);

    while (PC < MAX) {
        usleep(500000);
        PC++;

        char dev = '-';
        char op = '-';
        char state[16] = "RUNNING";

        // 15% chance de syscall
        int r = rand() % 100;
        if (r < 15) {  
            dev = (rand() % 2) ? '1' : '2';
            op = "RWX"[rand() % 3];
            printf("[App %d] -> syscall(D%c, %c)\n", pid, dev, op);
        }

        if(PC >= MAX) {
            strcpy(state, "TERMINATED");
        }

        char msg[64];
        sprintf(msg, "%d %c %c %d %s\n", pid, dev, op, PC, state);
        write(fd, msg, strlen(msg));

        printf("[App %d] - PC=%d executando...\n", pid, PC);
        usleep(500000);
    }

    printf("[App %d] - Finalizado (PC=%d)\n", pid, PC);
    close(fd);
    return 0;
}
