#ifndef QUEUE_H
#define QUEUE_H

#include <sys/types.h>  // para pid_t

void enqueue(pid_t queue[], int *n, pid_t pid);
pid_t dequeue(pid_t queue[], int *n);

#endif
