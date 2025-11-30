#ifndef UDP_REP_H
#define UDP_REP_H

typedef struct {
    int error;
    int owner;
    char op[8];
    char opType; 
    int payloadLen;
    char payload[256];
} udp_rep;

#endif