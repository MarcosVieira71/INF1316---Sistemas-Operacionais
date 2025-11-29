#ifndef KERNEL_REPLY_H
#define KERNEL_REPLY_H

#include "udp_rep.h"
#include <string.h>

typedef struct {
    udp_rep rep;   // a resposta UDP bruta
    char op[4];    // "RD", "WR", "DC", "DR", "DL"
} kernel_reply;

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


#endif