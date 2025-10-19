#ifndef KERNEL_FUNCTIONS_H
#define KERNEL_FUNCTIONS_H

#include "process.h"
#include "queue.h"
#include <unistd.h>

int timeSlice(int *current, Process* processes, int num_proc);
int releaseDevice(pid_t queue[], int *n, Process* processes, int num_proc);

void handleSyscallMessage(pid_t pid, char dev, char op,
                          Process* processes, int num_proc,
                          pid_t queue_D1[], int *n_D1,
                          pid_t queue_D2[], int *n_D2);

void handleIrqFifo(char buf, int *current, Process* processes, int num_proc,
                  pid_t queue_D1[], int* n_D1, 
                  pid_t queue_D2[], int* n_D2);

void printProcessStates(Process* processes, int num_proc);

int allProcessesTerminated(Process* processes, int num_proc);

#endif
