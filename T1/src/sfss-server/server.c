#include "server/server_functions.h"
#include "udp_msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define ROOT_DIR "SFS-root-dir"

int main(void) {
    ensureDirExists(ROOT_DIR);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(6000);

    bind(sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
    printf("[SFS] Servidor de gerência de arquivos inicializado\n");

    while(1)
    {
        udp_msg req;
        struct sockaddr_in cliAddr;
        socklen_t cliLen = sizeof(cliAddr);

        int n = recvfrom(sock, &req, sizeof(req), 0, (struct sockaddr*)&cliAddr, &cliLen);

        if(n <= 0) continue;

        printf("[SFS] Recebido request op=%s owner=%d\n", req.op, req.owner);

        udp_msg rep;
        memset(&rep, 0, sizeof(rep));

        rep.owner = req.owner;
        strncpy(rep.op, req.op, sizeof(rep.op) - 1);

        handleOperation(&req, &rep);
        sendto(sock, &rep, sizeof(rep), 0, (struct sockaddr*)&cliAddr, cliLen);
    }
}


