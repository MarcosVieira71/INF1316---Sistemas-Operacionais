#include "kernel_functions.h"
#include "queue.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int timeSlice(int *current, Process* processes, int num_proc) {
    if(processes[*current].state == RUNNING) {
        kill(processes[*current].pid, SIGSTOP);
        processes[*current].state = READY;
    }

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

    if(dev == '1') enqueue(queue_D1, n_D1, pid);
    else if(dev == '2') enqueue(queue_D2, n_D2, pid);
}


int handleIrqFifo(char buf, int *current, Process* processes, int num_proc,
                  pid_t queue_D1[], int n_D1, pid_t queue_D2[], int n_D2 )
{
    switch(buf) {
        case '0': {
            int running = timeSlice(current, processes, num_proc);
            if(running != -1)
                printf("[Kernel] - Processo %d agora RUNNING\n", running + 1);
            break;
        }
        case '1': {
            int idx = releaseDevice(queue_D1, &n_D1, processes, num_proc);
            if(idx >= 0)
                printf("[Kernel] - Processo %d desbloqueado do dispositivo D1\n", idx + 1);
            break;
        }
        case '2': {
            int idx = releaseDevice(queue_D2, &n_D2, processes, num_proc);
            if(idx >= 0)
                printf("[Kernel] - Processo %d desbloqueado do dispositivo D2\n", idx + 1);
            break;
        }
    }
    return 0;
}
