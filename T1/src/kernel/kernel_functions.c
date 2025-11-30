#include "kernel_functions.h"
#include "queue.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "shm_msg.h"

//A cada timeslice, interrompe o processo atual e retoma o próximo processo em estado READY
int timeSlice(int *current, Process* processes, int num_proc) {
    if (*current < num_proc && *current >= 0) {
        if(processes[*current].state == RUNNING) {
            kill(processes[*current].pid, SIGSTOP);
            if(processes[*current].state != TERMINATED)
                processes[*current].state = READY;
        }
    }
    //Fila circular de processos
    int next = (*current + 1) % num_proc;
    for(int i = 0; i < num_proc; i++) {
        if(processes[next].state == READY) {
            kill(processes[next].pid, SIGCONT);
            processes[next].state = RUNNING;
            *current = next;
            return next;
        }
        next = (next + 1) % num_proc;
    }
    return -1;
}

int releaseDevice(pid_t queue[], int *n, Process* processes, int num_proc) {
    pid_t pid = dequeue(queue, n);
    if(pid == -1) return -1;

    int idx = findProcessIndexByPid(processes, num_proc, pid);
    if(idx == -1) return -2;

    processes[idx].state = READY;
    processes[idx].blocked_on = 0;
    processes[idx].op = 0;
    return idx;
}

void handleSyscallMessage(int idx, Process* processes, int num_proc) {

    if (idx == -1) return;

    processes[idx].state = BLOCKED;

    // pausa imediatamente
    kill(processes[idx].pid, SIGSTOP);

    printf("[Kernel] Processo A%d bloqueado aguardando resposta do SFSS\n", idx+1);
}

void handleIrqFifo(char buf,
                   int *current,
                   Process* processes,
                   int num_proc,
                   shm_msg* shm[],
                   kernel_reply fileQueue[], int* nFile,
                   kernel_reply dirQueue[], int* nDir)
{
    switch(buf) {
        case '0': {
            int running = timeSlice(current, processes, num_proc);
            if(running != -1)
                printf("[Kernel] - Processo %d agora RUNNING\n", running + 1);
            break;
        }
        case '1':   // IRQ1
            deliverFileReply(processes, shm, fileQueue, nFile);
            break;

        case '2':   // IRQ2
            deliverDirReply(processes, shm, dirQueue, nDir);
            break;

    }
}

void printProcessStates(Process* processes, int num_proc) 
{
    printf("=== Estados dos Processos ===\n");

    for (int i = 0; i < num_proc; i++) {

        printf("Processo %d (PID=%d): PC=%d, Estado=%s",
            i+1, processes[i].pid, processes[i].PC,
            (processes[i].state == RUNNING) ? "RUNNING" :
            (processes[i].state == READY) ? "READY" :
            (processes[i].state == BLOCKED) ? "BLOCKED" :
            "TERMINATED"
        );
        printf("\n");
    }
    printf("============================\n");
}


int allProcessesTerminated(Process* processes, int num_proc) 
{
    for(int i = 0; i < num_proc; i++) {
        if(processes[i].state != TERMINATED)
            return 0;
    }
    return 1; 
}


void deliverFileReply(Process* processes,
                      shm_msg* shm[],
                      kernel_reply fileQueue[],
                      int *nFile)
{
    if (*nFile == 0)
        return; // nada para entregar

    // remove o mais antigo
    kernel_reply kr = dequeueReply(fileQueue, nFile);

    int owner = kr.rep.owner;   // 1..5
    int idx = owner - 1;

    // preencher shared memory do app correto
    shm[idx]->error = kr.rep.error;

    if (kr.rep.payloadLen > 0)
        memcpy(shm[idx]->payload, kr.rep.payload, kr.rep.payloadLen);

    shm[idx]->has_reply = 1;

    // desbloquear processo correspondente
    processes[idx].state = READY;

    printf("[Kernel] Entreguei FILE reply para A%d (op=%s)\n", owner, kr.op);
}

void deliverDirReply(Process* processes,
                     shm_msg* shm[],
                     kernel_reply dirQueue[],
                     int *nDir)
{
    if (*nDir == 0)
        return; // sem resposta a entregar

    kernel_reply kr = dequeueReply(dirQueue, nDir);

    int owner = kr.rep.owner;   // 1..5
    int idx = owner - 1;

    shm[idx]->error = kr.rep.error;

    if (kr.rep.payloadLen > 0)
        memcpy(shm[idx]->payload, kr.rep.payload, kr.rep.payloadLen);

    shm[idx]->has_reply = 1;

    processes[idx].state = READY;

    printf("[Kernel] Entreguei DIR reply para A%d (op=%s)\n", owner, kr.op);
}

void cleanOldShms(int numProc)
{
    for (int i = 0; i < numProc; i++) 
    {
        char name_shm[32];
        sprintf(name_shm, "/shm_A%d", i + 1);

        shm_unlink(name_shm);
    }
}


void closeShms(int numProc, shm_msg* shm[])
{
    for (int i = 0; i < numProc; i++) {
        if (shm[i] && shm[i] != MAP_FAILED) munmap(shm[i], sizeof(shm_msg));
        char name_shm[32];
        sprintf(name_shm, "/shm_A%d", i + 1);
        shm_unlink(name_shm);
    }
}
