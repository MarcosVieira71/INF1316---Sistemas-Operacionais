#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>

#include "fifo.h"

sig_atomic_t running_flag = 1;

void sigusr1Handler(int sig) {
    running_flag = 0;
}

int main() {
    int fd;

    signal(SIGUSR1, sigusr1Handler);

    if (openFIFO(&fd, FIFO_IRQ, O_WRONLY) == -1) {
        printf("Erro: openFIFO");
        exit(1);
    }

    printf("[InterController] - Iniciando geração de interrupções...\n");

    while (running_flag) {
        usleep(500000); // 500 ms

        /* IRQ0: sempre */
        if (write(fd, "0", 1) != 1) {
            printf("Erro: write IRQ0\n");
            break;
        }
        printf("[InterController] - IRQ0 (Time Slice)\n");

        /* IRQ1: probabilidade 10% */
        if ((rand() % 100) < 10) {
            if (write(fd, "1", 1) != 1) {
                printf("Erro: write IRQ1\n");
                break;
            }
            printf("[InterController] - IRQ1 (File response pronta)\n");
        }

        /* IRQ2: probabilidade 2% */
        if ((rand() % 100) < 2) {
            if (write(fd, "2", 1) != 1) {
                printf("Erro: write IRQ2\n");
                break;
            }
            printf("[InterController] - IRQ2 (Dir response )\n");
        }
    }

    close(fd);
    printf("[InterController] - Finalizando execução\n");
    return 0;
}
