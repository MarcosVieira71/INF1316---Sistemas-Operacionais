#include "kernel/kernel_reply.h"
#include <string.h>


void enqueueReply(kernel_reply queue[], int *n, kernel_reply item) {
    queue[*n] = item;
    (*n)++;
}

kernel_reply dequeueReply(kernel_reply queue[], int *n) {
    kernel_reply empty;
    memset(&empty, 0, sizeof(kernel_reply));

    if (*n == 0) return empty;

    kernel_reply r = queue[0];

    for (int i = 1; i < *n; i++)
        queue[i-1] = queue[i];

    (*n)--;

    return r;
}

