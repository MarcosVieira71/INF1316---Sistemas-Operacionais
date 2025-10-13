#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h> 
#include <fcntl.h>

#define FIFO_IRQ "/tmp/fifo_irq"
#define FIFO_SYSCALL "/tmp/fifo_syscall"
#define SIZE 10

typedef enum {RUNNING, READY, BLOCKED, TERMINATED} State;

typedef struct {
    pid_t pid;
    int PC;
    State state;
    char blocked_on;
} Process;

int startProcesses(Process* p, int n)
{
    for(int i = 0; i < n; i++){
        p[i].pid = 0; 
        p[i].PC = 0;
        p[i].state = READY;
        p[i].blocked_on = 0;  
    }
    return 0;
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
    int err = startProcesses(processes, 5);


    err = makeFIFOs();
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
    char buf[SIZE];

    while(1)
    {



    }
    
    printf("[Kernel] - FIFOs criadas com sucesso.\n");


}
