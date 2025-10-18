#include "queue.h"

void enqueue(pid_t queue[], int *n, pid_t pid) {
    queue[*n] = pid;
    (*n)++;
}

pid_t dequeue(pid_t queue[], int *n) {
    if(*n == 0) return -1;

    pid_t pid = queue[0];
    for(int i = 1; i < *n; i++)
        queue[i-1] = queue[i];
    (*n)--;
    return pid;
}
