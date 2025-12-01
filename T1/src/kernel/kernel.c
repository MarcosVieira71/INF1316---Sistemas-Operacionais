#include "kernel/kernel_functions.h"
#include "kernel/kernel_reply.h"
#include "fifo.h"
#include "process.h"
#include "queue.h"
#include "shm_msg.h"
#include "kernel/udp_client_functions.h"
#include "udp_msg.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>


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

    kernel_reply fileQueue[NUM_PROC];
    int nFile = 0;

    kernel_reply dirQueue[NUM_PROC];
    int nDir = 0;

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
    cleanOldShms(NUM_PROC);

    startProcesses(processes, NUM_PROC);

    shm_msg *shm[NUM_PROC];
    // Cria uma shared memory para cada processo

    for (int i = 0; i < NUM_PROC; i++)
    {
        char name_shm[32];

        sprintf(name_shm, "/shm_A%d", i + 1);

        int fd = shm_open(name_shm, O_CREAT | O_RDWR, 0666);

        if (fd < 0)
        {
            perror("shm_open");
            exit(1);
        }

        ftruncate(fd, sizeof(shm_msg)); // define o tamanho do objeto de shared memory

        shm[i] = mmap(NULL, sizeof(shm_msg), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        memset(shm[i], 0, sizeof(shm_msg));

        close(fd);
    }

    struct sockaddr_in serverAddr;
    int udpSock = createUdpSocket("127.0.0.1", 6000, &serverAddr);

    if (udpSock < 0) {
        fprintf(stderr, "[Kernel] Erro ao criar socket UDP\n");
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
            char ownerStr[16];
            sprintf(ownerStr, "%d", i + 1);
            signal(SIGINT, SIG_IGN);
            execl("./app", "./app", ownerStr, NULL);
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
            handleIrqFifo(irq_buf, 
                        &current,
                        processes,
                        NUM_PROC,
                        shm,
                        fileQueue, &nFile,
                        dirQueue, &nDir);
        }

        kernel_reply reply = recvUdpReply(udpSock, shm, processes);

        if(reply.valid)
        {
            if(!strcmp(reply.op, "RD") || !strcmp(reply.op, "WR")) 
            {
                enqueueReply(fileQueue, &nFile, reply);
            }
            else enqueueReply(dirQueue, &nDir, reply);
        }

        handleProcessRequests(processes, NUM_PROC, shm, udpSock, &serverAddr);

        handlePauseAndResume(pause_flag, processes, NUM_PROC, intercontroller);

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

    if(udpSock >= 0) close(udpSock);

    closeShms(NUM_PROC, shm);

    return 0;
}
