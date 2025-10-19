#include "kernel_functions.h"
#include "fifo.h"
#include "process.h"
#include "queue.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


#define NUM_PROC 5

sig_atomic_t pause_flag = 0;

void sigintHandler(int sig) {
    pause_flag = !pause_flag;
}

void sigtstpHandler(int sig) {
    printf("[Kernel] - Recebido SIGTSTP (Ctrl+Z), encerrando todos os processos...\n");
    kill(0, SIGTERM);
    exit(0);
}

int main() {
    Process processes[NUM_PROC];
    pid_t intercontroller;

    pid_t queue_D1[NUM_PROC];
    int n_D1 = 0;
    pid_t queue_D2[NUM_PROC];
    int n_D2 = 0;

    signal(SIGINT, sigintHandler);
    signal(SIGTSTP, sigtstpHandler);

    if(makeFIFO(FIFO_IRQ) == -1 || makeFIFO(FIFO_SYSCALL) == -1) {
        exit(1);
    }

    int fd_irq, fd_syscall;
    if(openFIFO(&fd_irq, FIFO_IRQ, O_RDONLY | O_NONBLOCK) == -1 ||
       openFIFO(&fd_syscall, FIFO_SYSCALL, O_RDONLY | O_NONBLOCK) == -1) {
        exit(1);
    }
    
    intercontroller = fork();
    if(intercontroller < 0){
        exit(1);
    }
    else if(intercontroller == 0)
    {
        signal(SIGINT, SIG_IGN); 
        execl("./intercontroller", "./intercontroller", NULL);
        exit(1);
    }
    
    kill(intercontroller, SIGSTOP);

    startProcesses(processes, NUM_PROC);

    for(int i = 0; i < NUM_PROC; i++) {
        pid_t pid = fork();
        if(pid < 0) {
            exit(1);
        } else if(pid == 0) {
            signal(SIGINT, SIG_IGN); 
            execl("./app", "./app", NULL);
            exit(1);
        } else {
            processes[i].pid = pid;
            kill(processes[i].pid, SIGSTOP);
        }
    }

    printf("[Kernel] - Kernel inicializado.\n");

    kill(intercontroller, SIGCONT);


    int current = -1;
    char irq_buf;
    char syscall_msg[32];

    while(1) {
        // IRQ
        if(read(fd_irq, &irq_buf, 1) > 0)
        {
            handleIrqFifo(irq_buf, &current, processes, NUM_PROC,
                          queue_D1, &n_D1, queue_D2, &n_D2);
        }

        //Syscall
        int n = read(fd_syscall, syscall_msg, sizeof(syscall_msg));
        if(n > 0) {
            pid_t pid;
            char dev, op;
            int pc;
            char state[16];

            if(sscanf(syscall_msg, "%d %c %c %d %s", &pid, &dev, &op, &pc, state) == 5) {
                int idx = findProcessIndexByPid(processes, NUM_PROC, pid);
                if(idx == -1) return -2;
                processes[idx].PC = pc;

                if(strcmp(state, "TERMINATED") == 0) {
                    processes[idx].state = TERMINATED;
                } else if(dev != '-') {
                    handleSyscallMessage(pid, dev, op, processes, NUM_PROC,
                                        queue_D1, &n_D1, queue_D2, &n_D2);
                }
            }
        }
        
        if(pause_flag) {
            printProcessStates(processes, NUM_PROC);
            for(int i = 0; i < NUM_PROC; i++)
            {
                kill(processes[i].pid, SIGSTOP);
            }
            kill(intercontroller, SIGSTOP);
            pause();
        }
        else{
            for(int i = 0; i < NUM_PROC; i++)
            {
                if(processes[i].state == RUNNING){
                    kill(processes[i].pid, SIGCONT);
                }
            }
            kill(intercontroller, SIGCONT);
        }

        if(allProcessesTerminated(processes, NUM_PROC)){
            break;
        }
    }
    kill(intercontroller, SIGUSR1);
    waitpid(0, NULL, 0);
    printProcessStates(processes, NUM_PROC);
    printf("[Kernel] - Kernel finalizando. Todos os processos acabaram sua execução\n");
    close(fd_irq);
    close(fd_syscall);
    
    unlink(FIFO_IRQ);
    unlink(FIFO_SYSCALL);
    
    return 0;
}
