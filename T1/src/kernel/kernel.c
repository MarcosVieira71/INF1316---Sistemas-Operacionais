#include "kernel_functions.h"
#include "fifo.h"
#include "process.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define NUM_PROC 5

int main() {

    pid_t queue_D1[NUM_PROC];
    int n_D1 = 0;
    pid_t queue_D2[NUM_PROC];
    int n_D2 = 0;

    Process processes[NUM_PROC];
    startProcesses(processes, NUM_PROC);

    if(makeFIFO(FIFO_IRQ) == -1 || makeFIFO(FIFO_SYSCALL) == -1) {
        exit(1);
    }

    int fd_irq, fd_syscall;
    if(openFIFO(&fd_irq, FIFO_IRQ, O_RDONLY |  O_NONBLOCK) == -1 ||
       openFIFO(&fd_syscall, FIFO_SYSCALL, O_RDONLY |  O_NONBLOCK) == -1) {
        exit(1);
    }

    printf("[Kernel] - Kernel inicializado.\n");


    int current = 0;
    char irq_buf;
    char syscall_msg[32];

    while(1) {
        // IRQ
        if(read(fd_irq, &irq_buf, 1) > 0) {
            handleIrqFifo(irq_buf, &current, processes, NUM_PROC,
                          queue_D1, n_D1, queue_D2, n_D2);
        }

        //Syscall
        int n = read(fd_syscall, syscall_msg, sizeof(syscall_msg));
        if(n > 0) {
            pid_t pid;
            char dev, op;
            if(sscanf(syscall_msg, "%d %c %c", &pid, &dev, &op) == 3) {
                handleSyscallMessage(pid, dev, op, processes, NUM_PROC,
                                    queue_D1, &n_D1, queue_D2, &n_D2);
                printf("[Kernel] - Processo %d bloqueado em D%c (operação %c)\n",
                    findProcessIndexByPid(processes, NUM_PROC, pid)+1, dev, op);
            }
        }
    }

    return 0;
}
