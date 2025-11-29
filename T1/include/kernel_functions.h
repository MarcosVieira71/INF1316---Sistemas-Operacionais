#ifndef KERNEL_FUNCTIONS_H
#define KERNEL_FUNCTIONS_H

#include "process.h"
#include "queue.h"
#include "kernel_reply.h"

#include <unistd.h>

int timeSlice(int *current, Process* processes, int num_proc);
int releaseDevice(pid_t queue[], int *n, Process* processes, int num_proc);

void handleSyscallMessage(size_t idx, Process* processes, int num_proc);

void deliverFileReply(Process* processes,
                      shm_msg* shm[],
                      kernel_reply fileQueue[],
                      int *nFile);


void deliverDirReply(Process* processes,
                     shm_msg* shm[],
                     kernel_reply dirQueue[],
                     int *nDir);

void handleIrqFifo(char buf,
                   int *current,
                   Process* processes,
                   int num_proc,
                   shm_msg* shm[],
                   kernel_reply fileQueue[], int* nFile,
                   kernel_reply dirQueue[], int* nDir);


void printProcessStates(Process* processes, int num_proc);

int allProcessesTerminated(Process* processes, int num_proc);

#endif
