#include "process.h"
#include <stdio.h>
#include <string.h>

void startProcesses(Process* p, int n)
{
    for(int i = 0; i < n; i++){
        p[i].pid = 0; 
        p[i].PC = 0;
        p[i].state = READY;
        strcpy(p[i].op, "0");
    }
}

int findProcessIndexByPid(Process* processes, int num, pid_t pid)
{
    for(int i = 0; i < num; i++){
        if(processes[i].pid == pid)
            return i;
    }
    return -1;
}
