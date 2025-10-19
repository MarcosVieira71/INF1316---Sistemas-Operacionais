#include "fifo.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>


sig_atomic_t running_flag = 1;

void sigusr1Handler(int sig) {
    running_flag = 0;
}

int main() {
    int fd;
    signal(SIGUSR1, sigusr1Handler);

    srand(time(NULL));


    if(openFIFO(&fd, FIFO_IRQ, O_WRONLY) == -1){
        exit(1);
    };

    printf("[InterController] - Iniciando geração de interrupções...\n");

    while (running_flag) {
        usleep(500000); // 500ms = time slice

        // IRQ0: sempre ocorre a cada 500 ms
        write(fd, "0", 1);
        printf("[InterController] - IRQ0 (Time Slice)\n");

        // IRQ1: probabilidade de 10%
        if ((rand() % 100) < 10) {
            write(fd, "1", 1);
            printf("[InterController] - IRQ1 (D1 terminou I/O)\n");
        }

        // IRQ2: probabilidade de 5%
        if ((rand() % 100) < 5) {
            write(fd, "2", 1);
            printf("[InterController] - IRQ2 (D2 terminou I/O)\n");
        }
    }
    close(fd);
    printf("[InterController] - Finalizando execução\n");
    return 0;
}
