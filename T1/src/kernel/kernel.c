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

    if (makeFIFO(FIFO_IRQ) == -1)
    {
        exit(1);
    }

    int fd_irq;
    if (openFIFO(&fd_irq, FIFO_IRQ, O_RDONLY | O_NONBLOCK) == -1)
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

    shm_msg *shm[NUM_PROC];

    // Cria uma shared memory para cada processo

    for (int i = 0; i < NUM_PROC; i++)
    {
        char name_shm[32];

        sprintf(name_shm, "/shm_a%d", i + 1);

        int fd = shm_open(name_shm, O_CREAT | O_RDWR, 0666);

        if (fd < 0)
        {
            perror("shm_open");
            exit(1);
        }

        ftruncate(fd, sizeof(shm_msg));

        shm[i] = mmap(NULL, sizeof(shm_msg), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        memset(shm[i], 0, sizeof(shm_msg));

        close(fd);
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

    while (1)
    {
        // IRQ
        if (read(fd_irq, &irq_buf, 1) > 0)
        {
            handleIrqFifo(irq_buf, &current, processes, NUM_PROC,
                          queue_D1, &n_D1, queue_D2, &n_D2);
        }

        for (int i = 0; i < NUM_PROC; i++)
        {
            if (shm[i]->has_request)
            {

                printf("[Kernel] Recebido request do processo A%d:\n", i + 1);
                printf("op = %s\n", shm[i]->op);
                printf("path = %s\n", shm[i]->path);
                printf("offset = %d\n", shm[i]->offset);
                printf("owner = %d\n", shm[i]->owner);

                shm[i]->has_request = 0;
                kill(processes[i].pid, SIGSTOP);
                processes[i].state = BLOCKED;
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

    unlink(FIFO_IRQ);

    return 0;
}
