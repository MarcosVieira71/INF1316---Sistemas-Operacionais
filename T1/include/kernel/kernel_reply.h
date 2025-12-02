#ifndef KERNEL_REPLY_H
#define KERNEL_REPLY_H

#include "udp_msg.h"
#include <string.h>

typedef struct {
    udp_msg rep;   // a resposta UDP bruta
    char op[4];    // "RD", "WR", "DC", "DR", "DL"
    int valid;     // 0 para reply inválida e 1 para válida
} kernel_reply;

void enqueueReply(kernel_reply queue[], int *n, kernel_reply item);

kernel_reply dequeueReply(kernel_reply queue[], int *n);

void print_kernel_response(kernel_reply *reply);

#endif