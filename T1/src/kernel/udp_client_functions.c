#include "shm_msg.h"
#include "udp_msg.h"
#include "kernel/kernel_reply.h"
#include "kernel/udp_client_functions.h"
#include "process.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int createUdpSocket(const char* serverIp, int serverPort, struct sockaddr_in* srvAddr)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) 
    {
        perror("socket");
        return -1;
    }

    memset(srvAddr, 0, sizeof(*srvAddr));
    srvAddr->sin_family = AF_INET;
    srvAddr->sin_port = htons(serverPort);

    if (inet_aton(serverIp, &srvAddr->sin_addr) == 0) {
        fprintf(stderr, "serverIp invalido\n");
        close(sockfd);
        return -1;
    }

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);


    return sockfd;
}

void buildReqFromShm(udp_msg* req, const shm_msg* shm) 
{
    memset(req, 0, sizeof(*req));
    strncpy(req->op, shm->op, sizeof(req->op)-1);
    req->owner = shm->owner;
    req->offset = shm->offset;

    req->pathLen = (int)strnlen(shm->path, sizeof(shm->path));
    if (req->pathLen > 0) memcpy(req->path, shm->path, req->pathLen);

    req->dirnameLen = (int)strnlen(shm->dirname, sizeof(shm->dirname));
    if (req->dirnameLen > 0) memcpy(req->dirname, shm->dirname, req->dirnameLen);

    req->payloadLen = (shm->payloadLen > sizeof(req->payload)) ? sizeof(req->payload) : shm->payloadLen;

    memcpy(req->payload, shm->payload, req->payloadLen);
}

int sendUdpRequest(int sockfd, struct sockaddr_in* srvAddr, const udp_msg* req)
{
    int n = sendto(sockfd, req, sizeof(udp_msg), 0,
                       (struct sockaddr*)srvAddr, sizeof(*srvAddr));
    if (n < 0) 
    { 
        perror("sendto"); 
        return -1; 
    }

    return 0;
}

kernel_reply recvUdpReply(int sockfd, shm_msg* shm[], Process processes[], char written_files[NUM_PROC][MAX_WRITTEN_FILES][MAX_PATH_LEN], int written_files_count[NUM_PROC])
{
    udp_msg response;
    struct sockaddr_in src;
    socklen_t slen = sizeof(src);

    int n = recvfrom(sockfd, &response, sizeof(response), 0, (struct sockaddr*)&src, &slen);

    kernel_reply reply = {0};

    if(n < 0)
    {
        // Response pode ser vazia porque a socket é non-block
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            reply.valid = 0;
            return reply;
        }
         // Erro real
        perror("recvfrom");    
        reply.valid = 0;
        return reply;
    }

    reply.valid = 1;

    int idx = response.owner - 1;

    shm[idx]->error = response.error;
    shm[idx]->has_reply = 1;

    if (response.payloadLen > 0) memcpy(shm[idx]->payload, response.payload, response.payloadLen);

    reply.rep = response;
    strcpy(reply.op, response.op);
    if(strcmp(reply.op, "WR") == 0)
    {
        if(!response.isDeleting && response.offset >= 0)
        {
            int pos = written_files_count[idx];  // última posição usada
    
            if (pos < MAX_WRITTEN_FILES)
            {
                strncpy(written_files[idx][pos], response.path, MAX_PATH_LEN - 1);
    
                written_files_count[idx]++;   // atualiza contador
            }
        }
        else if(response.isDeleting)
        {
            int count = written_files_count[idx];
            for (int i = 0; i < count; i++)
            {
                if (strncmp(written_files[idx][i], response.path, MAX_PATH_LEN) == 0)
                {
                    for (int j = i; j < count - 1; j++)
                    {
                        strncpy(written_files[idx][j],
                                written_files[idx][j + 1],
                                MAX_PATH_LEN);
                    }

                    written_files_count[idx]--;

                    break; 
                }
            }
        }
    }

    return reply;
}