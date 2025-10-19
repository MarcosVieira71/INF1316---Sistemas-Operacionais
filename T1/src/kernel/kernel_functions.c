#include "kernel_functions.h"
#include "queue.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

//A cada timeslice, interrompe o processo atual e retoma o próximo processo em estado READY
int timeSlice(int *current, Process* processes, int num_proc) {
    if(processes[*current].state == RUNNING) {
        kill(processes[*current].pid, SIGSTOP);
        if(processes[*current].state != TERMINATED)
            processes[*current].state = READY;
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
    return idx;
}

void handleSyscallMessage(pid_t pid, char dev, char op,
                          Process* processes, int num_proc,
                          pid_t queue_D1[], int *n_D1,
                          pid_t queue_D2[], int *n_D2)
{
    int idx = findProcessIndexByPid(processes, num_proc, pid);
    if(idx == -1) return;

    processes[idx].state = BLOCKED;
    processes[idx].blocked_on = dev;

    kill(pid, SIGSTOP);

    if(dev == '1'){
        enqueue(queue_D1, n_D1, pid);
        processes[idx].d1_accesses++;
    } 
    else if(dev == '2'){
        enqueue(queue_D2, n_D2, pid);
        processes[idx].d2_accesses++;
    } 
}


int handleIrqFifo(char buf, int *current, Process* processes, int num_proc,
                  pid_t queue_D1[], int* n_D1, pid_t queue_D2[], int* n_D2 )
{
    switch(buf) {
        case '0': {
            int running = timeSlice(current, processes, num_proc);
            if(running != -1)
                printf("[Kernel] - Processo %d agora RUNNING\n", running + 1);
            break;
        }
        case '1': {
            int idx = releaseDevice(queue_D1, n_D1, processes, num_proc);
            if(idx >= 0) {
                printf("[Kernel] - Processo %d desbloqueado do dispositivo D1\n", idx + 1);
            }
                
            break;
        }
        case '2': {
            int idx = releaseDevice(queue_D2, n_D2, processes, num_proc);
            if(idx >= 0) {
                printf("[Kernel] - Processo %d desbloqueado do dispositivo D2\n", idx + 1);
            }
  
            break;
        }
    }
    return 0;
}

void printProcessStates(Process* processes, int num_proc) 
{
    printf("=== Estados dos Processos ===\n");
    for(int i = 0; i < num_proc; i++) {
        printf("Processo %d (PID=%d): PC=%d, Estado=%s",
               i+1, processes[i].pid, processes[i].PC,
               (processes[i].state == RUNNING) ? "RUNNING" :
               (processes[i].state == READY) ? "READY" :
               (processes[i].state == BLOCKED) ? "BLOCKED" :
               "TERMINATED");
        if(processes[i].state == BLOCKED)
            printf(", Bloqueado em D%c", processes[i].blocked_on);
        printf(", D1: %d, D2: %d", processes[i].d1_accesses, processes[i].d2_accesses);
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