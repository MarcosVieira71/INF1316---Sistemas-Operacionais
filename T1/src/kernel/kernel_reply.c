#include "kernel/kernel_reply.h"
#include <string.h>
#include <stdio.h>

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

void print_kernel_response(kernel_reply *reply)
{
    if (!reply || !reply->valid) {
        return;
    }

    udp_msg *m = &reply->rep;

    printf("[Kernel] Response recebida do servidor:\n");
    printf("    op = %s\n", reply->op);
    printf("    owner = %d\n", m->owner);
    printf("    error = %d\n", m->error);
    printf("    path = %s\n", m->path);

    // READ ou WRITE
    if (!strcmp(reply->op, "RD") || !strcmp(reply->op, "WR"))
    {
        printf("    offset = %d\n", m->offset);
        printf("    payloadLen = %d\n", m->payloadLen);

        if (m->payloadLen > 0) {
            printf("    payload: \"");
            for (int i = 0; i < m->payloadLen; i++) {
                putchar(m->payload[i]);
            }
            printf("\"\n");
        }
    }
    // CREATE_DIR ou REMOVE_DIR
    else if (!strcmp(reply->op, "DC") || !strcmp(reply->op, "DR"))
    {
        printf("    dirname = %s\n", m->dirname);
    }
    // LIST_DIR
    else if (!strcmp(reply->op, "DL"))
    {
        printf("    listinfo.nrnames = %d\n", m->listinfo.nrnames);

        for (int i = 0; i < m->listinfo.nrnames; i++)
        {
            int start = m->listinfo.fstlstpositions[i][0];
            int end   = m->listinfo.fstlstpositions[i][1];

            printf("    [%d] %.*s  (%s)\n",
                   i,
                   end - start,
                   &m->listinfo.allnames[start],
                   m->listinfo.isDir[i] ? "DIR" : "FILE");
        }
    }

    printf("\n");
}
