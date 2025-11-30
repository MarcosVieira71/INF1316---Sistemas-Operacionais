#ifndef KERNEL_REPLY_H
#define KERNEL_REPLY_H

#include "udp_rep.h"
#include <string.h>

typedef struct {
    udp_rep rep;   // a resposta UDP bruta
    char op[4];    // "RD", "WR", "DC", "DR", "DL"
} kernel_reply;

void enqueueReply(kernel_reply queue[], int *n, kernel_reply item);

kernel_reply dequeueReply(kernel_reply queue[], int *n);


#endif