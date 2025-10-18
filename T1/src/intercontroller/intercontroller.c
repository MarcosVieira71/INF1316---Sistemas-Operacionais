#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define FIFO_IRQ "/tmp/fifo_irq"
#include "fifo.h"

int main() {
    int fd;
    srand(time(NULL));

    if(makeFIFO(FIFO_IRQ) == -1){
        exit(1);
    }

    if(openFIFO(&fd, FIFO_IRQ, O_WRONLY) == -1){
        exit(1);
    };

    printf("[InterController] - Iniciando geração de interrupções...\n");

    while (1) {
        usleep(500000); // 500ms = time slice

        // IRQ0: sempre ocorre a cada 500 ms
        write(fd, "0", 1);
        printf("[InterController] -> IRQ0 (Time Slice)\n");

        // IRQ1: probabilidade de 10%
        if ((rand() % 100) < 10) {
            write(fd, "1", 1);
            printf("[InterController] -> IRQ1 (D1 terminou I/O)\n");
        }

        // IRQ2: probabilidade de 5%
        if ((rand() % 100) < 5) {
            write(fd, "2", 1);
            printf("[InterController] -> IRQ2 (D2 terminou I/O)\n");
        }
    }

    close(fd);
    return 0;
}
