#ifndef SHM_MSG_H
#define SHM_MSG_H

typedef struct {
    int has_request;   // App → Kernel
    int has_reply;     // Kernel → App

    char op[8];        // "RD", "WR", "DC", "DR", "DL"
    char path[256];    // caminho completo
    char dirname[64];  // nome do diretório para DC/DR

    char payload[16];  // dados para WR ou recebidos no RD
    int payloadLen;

    int pc;            // program counter
    int offset;        // múltiplo de 16
    int owner;         // 1..5 (app A1..A5)
    int strlenPath;       // tamanho da string path
    int strlenDirName;       // tamanho da string dirname
    int error;         // código de erro (kernel → app)
} shm_msg;

#endif
