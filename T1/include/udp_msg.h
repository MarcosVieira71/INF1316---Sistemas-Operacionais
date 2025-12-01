#ifndef UDP_MSG_H
#define UDP_MSG_H

#define MAX_NAMES 32
#define MAX_ALLNAMES 512

typedef struct {
    char op[8];
    int error;
    int owner; // 1-5
    int offset;

    int pathLen;
    char path[256];

    int dirnameLen;
    char dirname[64];

    int payloadLen;
    char payload[16];

    struct {
        int nrnames;                          
        char allnames[MAX_ALLNAMES];         
        int fstlstpositions[MAX_NAMES][2];    
        int isDir[MAX_NAMES];                
    } listinfo;
} udp_msg;

#endif