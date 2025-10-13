#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h> 
#include <fcntl.h>

#include "queue.h"

#define FIFO_IRQ "/tmp/fifo_irq"
#define FIFO_SYSCALL "/tmp/fifo_syscall"
#define NUM_PROC 5

typedef enum {RUNNING, READY, BLOCKED, TERMINATED} State;

typedef struct {
    pid_t pid;
    int PC;
    State state;
    char blocked_on;
} Process;

void startProcesses(Process* p, int n)
{
    for(int i = 0; i < n; i++){
        p[i].pid = 0; 
        p[i].PC = 0;
        p[i].state = READY;
        p[i].blocked_on = 0;  
    }
}

void handleIRQ0(int *current, Process* processes) {
    if(processes[*current].state == RUNNING) {
        kill(processes[*current].pid, SIGSTOP);
        processes[*current].state = READY;
        printf("[Kernel] - Processo %d interrompido pelo IRQ0.\n", *current+1);
    }

    int next = (*current + 1) % NUM_PROC;
    for(int i = 0; i < NUM_PROC; i++) {
        if(processes[next].state == READY) {
            kill(processes[next].pid, SIGCONT);
            processes[next].state = RUNNING;
            *current = next;
            printf("[Kernel] - Processo %d agora RUNNING.\n", *current+1);
            break;
        }
        next = (next + 1) % NUM_PROC; 
    }

}

void handlerReleaseDevice(const char* device, pid_t fila[], int *n, Process processes[5]) {
    pid_t pid = dequeue(fila, n);  
    if(pid == -1) return;           

    for(int i = 0; i < 5; i++) {
        if(processes[i].pid == pid) {
            processes[i].state = READY;
            processes[i].blocked_on = 0;
            printf("[Kernel] - Processo %d desbloqueado do dispositivo %s. Agora está em READY\n", i, device);
            break;
        }
    }
}



int makeFIFOs(void){
    if (mkfifo(FIFO_IRQ, S_IRUSR | S_IWUSR) == -1) {
        if (errno == EEXIST) {
            printf("[Kernel] - FIFO %s já existe, usando mesmo assim.\n", FIFO_IRQ);
        } else {
            perror("[Kernel] - Erro ao criar FIFO_IRQ");
            return -1;
        }
    } else {
        printf("[Kernel] - FIFO criada: %s\n", FIFO_IRQ);
    }

    if (mkfifo(FIFO_SYSCALL,  S_IRUSR | S_IWUSR) == -1) {
        if (errno == EEXIST) {
            printf("[Kernel] - FIFO %s já existe.\n", FIFO_SYSCALL);
        } else {
            perror("[Kernel] - Erro ao criar FIFO_SYSCALL.");
            return -1;
        }
    } else {
        printf("[Kernel] - FIFO criada: %s.\n", FIFO_SYSCALL);
    }

    printf("[Kernel] - FIFOs prontas para uso.\n");
    return 0;
}

int openFIFOs(int* fd_irq, int* fd_syscall)
{
    int irq = open(FIFO_IRQ, O_RDONLY);
    if(irq == -1){
        perror("Erro ao abrir FIFO_IRQ.");
        return -1;
    }

    int syscall = open(FIFO_SYSCALL, O_RDONLY);
    if(syscall == -1){
        perror("Erro ao abrir FIFO_SYSCALL.");
        return -1;
    }
    *fd_irq = irq;
    *fd_syscall = syscall;
    return 0;
}


int main() {

    pid_t fila_D1[5];
    int n_D1 = 0;

    pid_t fila_D2[5];
    int n_D2 = 0;


    Process processes[5];
    startProcesses(processes, 5);


    int err = makeFIFOs();
    if(err)
    {
        perror("[Kernel] - Erro na criacao das FIFOs. \n");
        exit(1);
    }

    int fd_irq;
    int fd_syscall;

    err = openFIFOs(&fd_irq, &fd_syscall);
    if(err)
    {
        perror("[Kernel] - Erro na abertura das FIFOs. \n");
        exit(1);
    }
    
    int current = 0;
    char buf;

    while(1)
    {
        int n = read(fd_irq, &buf, 1);
        if(n > 0) {
            switch(buf) {
                case '0': // IRQ0
                    handleIRQ0(&current, processes);
                    break;
                case '1': // D1 (IRQ1)
                    handlerReleaseDevice("D1", fila_D1, &n_D1, processes);
                    break;
                case '2': // D2 (IRQ2)
                    handlerReleaseDevice("D2", fila_D2, &n_D2, processes);
                    break;
            }
        }
        
        n = read(fd_syscall, &buf, 1);        
        if(n > 0) {

        }

    }
    
    printf("[Kernel] - FIFOs criadas com sucesso.\n");


}
