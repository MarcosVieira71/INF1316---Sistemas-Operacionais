#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

typedef enum {RUNNING, READY, BLOCKED, TERMINATED} State;

typedef struct {
    pid_t pid;
    int PC;
    State state;
    char blocked_on;
} Process;

void startProcesses(Process* p, int n);
int findProcessIndexByPid(Process* processes, int num, pid_t pid);

#endif
