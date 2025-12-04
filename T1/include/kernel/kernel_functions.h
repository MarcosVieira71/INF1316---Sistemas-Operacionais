#ifndef KERNEL_FUNCTIONS_H
#define KERNEL_FUNCTIONS_H

#include "process.h"
#include "queue.h"
#include "kernel_reply.h"
#include "shm_msg.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>


#define NUM_PROC 5
#define MAX_WRITTEN_FILES 128
#define MAX_PATH_LEN 256

int timeSlice(int *current, Process* processes, int num_proc);
int releaseDevice(pid_t queue[], int *n, Process* processes, int num_proc);

void handleProcessRequests(Process *processes,
                           int numProc,
                           shm_msg **shm,
                           int udpSock,
                           struct sockaddr_in *serverAddr);

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
void printResponseQueues(kernel_reply* fileQueue, int nFile, kernel_reply* dirQueue, int nDir);

int allProcessesTerminated(Process* processes, int num_proc);

void cleanOldShms(int num_proc);

void closeShms(int num_proc, shm_msg* shm[]);

void handlePauseAndResume(int pause_flag,
                          Process *processes,
                          int num_proc,
                          kernel_reply* fileQueue, int nFile, kernel_reply* dirQueue, int nDir,
                          pid_t intercontroller,
                          char written_files[NUM_PROC][MAX_WRITTEN_FILES][MAX_PATH_LEN],
                          int written_files_count[NUM_PROC]);

void checkTerminatedProcesses(Process *processes, int num_proc);

void print_written_files(char written_files[NUM_PROC][MAX_WRITTEN_FILES][MAX_PATH_LEN], int written_files_count[NUM_PROC]);

#endif
