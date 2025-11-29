#include "kernel_functions.h"
#include "fifo.h"
#include "process.h"
#include "queue.h"
#include "shm_msg.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define NUM_PROC 5

sig_atomic_t pause_flag = 0;

void sigintHandler(int sig)
{
    pause_flag = !pause_flag;
}

void sigtstpHandler(int sig)
{
    printf("[Kernel] - Recebido SIGTSTP (Ctrl+Z), encerrando todos os processos...\n");
    kill(0, SIGTERM);
    exit(0);
}

int main()
{
    Process processes[NUM_PROC];
    pid_t intercontroller;

    // Filas para processos esperando nos dispositivos D1 e D2
    pid_t queue_D1[NUM_PROC];
    int n_D1 = 0;
    pid_t queue_D2[NUM_PROC];
    int n_D2 = 0;

    signal(SIGINT, sigintHandler);
    signal(SIGTSTP, sigtstpHandler);

    if (makeFIFO(FIFO_IRQ) == -1 || makeFIFO(FIFO_SYSCALL) == -1)
    {
        exit(1);
    }

    int fd_irq, fd_syscall;
    if (openFIFO(&fd_irq, FIFO_IRQ, O_RDONLY | O_NONBLOCK) == -1 ||
        openFIFO(&fd_syscall, FIFO_SYSCALL, O_RDONLY | O_NONBLOCK) == -1)
    {
        exit(1);
    }

    intercontroller = fork();
    if (intercontroller < 0)
    {
        exit(1);
    }
    else if (intercontroller == 0)
    {
        signal(SIGINT, SIG_IGN);
        execl("./intercontroller", "./intercontroller", NULL);
        exit(1);
    }

    // Pausa o InterController até que os processos de app estejam inicializados

    kill(intercontroller, SIGSTOP);

    startProcesses(processes, NUM_PROC);

    shm_msg *req_shm[NUM_PROC];
    shm_msg *rep_shm[NUM_PROC];

    // Cria duas shared memories Ax - kernel: uma para requests e outra para replies

    for (int i = 0; i < NUM_PROC; i++)
    {
        char name_req[32];
        char name_rep[32];

        sprintf(name_req, "/shm_req_%d", i + 1);
        sprintf(name_rep, "/shm_rep_%d", i + 1);

        int fd_req = shm_open(name_req, O_CREAT | O_RDWR, 0666);
        int fd_rep = shm_open(name_rep, O_CREAT | O_RDWR, 0666);
        if (fd_req < 0 || fd_rep < 0)
        {
            perror("shm_open");
            exit(1);
        }

        ftruncate(fd_req, sizeof(shm_msg));
        ftruncate(fd_rep, sizeof(shm_msg));

        req_shm[i] = mmap(NULL, sizeof(shm_msg), PROT_READ | PROT_WRITE, MAP_SHARED, fd_req, 0);
        rep_shm[i] = mmap(NULL, sizeof(shm_msg), PROT_READ | PROT_WRITE, MAP_SHARED, fd_rep, 0);

        memset(req_shm[i], 0, sizeof(shm_msg));
        memset(rep_shm[i], 0, sizeof(shm_msg));

        close(fd_req);
        close(fd_rep);
    }

    for (int i = 0; i < NUM_PROC; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            exit(1);
        }
        else if (pid == 0)
        {
            signal(SIGINT, SIG_IGN);
            execl("./app", "./app", NULL);
            exit(1);
        }
        else
        {
            processes[i].pid = pid;
            // Pausa os processos de app logo após a criação
            kill(processes[i].pid, SIGSTOP);
        }
    }

    printf("[Kernel] - Kernel inicializado.\n");

    // Após os processos em app estarem prontos, continua o InterController
    kill(intercontroller, SIGCONT);

    int current = -1;
    char irq_buf;
    char syscall_msg[32];

    while (1)
    {
        // IRQ
        if (read(fd_irq, &irq_buf, 1) > 0)
        {
            handleIrqFifo(irq_buf, &current, processes, NUM_PROC,
                          queue_D1, &n_D1, queue_D2, &n_D2);
        }

        // Syscall
        int n = read(fd_syscall, syscall_msg, sizeof(syscall_msg));
        if (n > 0)
        {
            pid_t pid;
            char dev, op;
            int pc;
            char state[16];

            if (sscanf(syscall_msg, "%d %c %c %d %s", &pid, &dev, &op, &pc, state) == 5)
            {
                int idx = findProcessIndexByPid(processes, NUM_PROC, pid);
                // Deve sempre encontrar o processo
                if (idx == -1)
                {
                    printf("Erro: processo não encontrado");
                    exit(1);
                }
                processes[idx].PC = pc;

                if (strcmp(state, "TERMINATED") == 0)
                {
                    processes[idx].state = TERMINATED;
                    kill(processes[idx].pid, SIGCONT);
                }
                else if (dev != '-')
                {
                    handleSyscallMessage(pid, dev, op, processes, NUM_PROC,
                                         queue_D1, &n_D1, queue_D2, &n_D2);
                }
            }
        }

        for (int i = 0; i < NUM_PROC; i++)
        {
            if (req_shm[i]->has_request)
            {

                printf("[Kernel] Recebido request do processo A%d:\n", i + 1);
                printf("op = %s\n", req_shm[i]->op);
                printf("path = %s\n", req_shm[i]->path);
                printf("offset = %d\n", req_shm[i]->offset);
                printf("owner = %d\n", req_shm[i]->owner);

                req_shm[i]->has_request = 0;
            }
        }

        // Ao pressionar Ctrl+C, imprime informações sobre os processos e pausa
        if (pause_flag)
        {
            printProcessStates(processes, NUM_PROC);
            for (int i = 0; i < NUM_PROC; i++)
            {
                kill(processes[i].pid, SIGSTOP);
            }
            kill(intercontroller, SIGSTOP);
            pause();
        }
        // Retoma execução dos processos em estado RUNNING e o InterController
        else
        {
            for (int i = 0; i < NUM_PROC; i++)
            {
                if (processes[i].state == RUNNING)
                {
                    kill(processes[i].pid, SIGCONT);
                }
            }
            kill(intercontroller, SIGCONT);
        }

        if (allProcessesTerminated(processes, NUM_PROC))
        {
            break;
        }
    }

    // Envia sinal para finalizar o InterController
    kill(intercontroller, SIGUSR1);

    for (int i = 0; i < NUM_PROC; i++)
    {
        waitpid(processes[i].pid, NULL, 0);
    }
    waitpid(intercontroller, NULL, 0);

    printProcessStates(processes, NUM_PROC);
    printf("[Kernel] - Kernel finalizando. Todos os processos acabaram sua execução\n");
    close(fd_irq);
    close(fd_syscall);

    unlink(FIFO_IRQ);
    unlink(FIFO_SYSCALL);

    return 0;
}
