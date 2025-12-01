#ifndef UDP_MSG_H
#define UDP_MSG_H

typedef struct {
    char op[8];
    int owner; // 1-5
    int offset;

    int pathLen;
    char path[256];

    int dirnameLen;
    char dirname[64];

    int payloadLen;
    char payload[16];
} udp_msg;

#endif