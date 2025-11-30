#include "shm_msg.h"
#include "udp_req.h"
#include "udp_rep.h"
#include "udp_client.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

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

void buildReqFromShm(udp_req* req, const shm_msg* shm) 
{
    memset(req, 0, sizeof(*req));
    strncpy(req->op, shm->op, sizeof(req->op)-1);
    req->owner = shm->owner;
    req->offset = shm->offset;

    req->pathLen = (int)strnlen(shm->path, sizeof(shm->path));
    if (req->pathLen > 0) memcpy(req->path, shm->path, req->pathLen);

    req->dirnameLen = (int)strnlen(shm->dirname, sizeof(shm->dirname));
    if (req->dirnameLen > 0) memcpy(req->dirname, shm->dirname, req->dirnameLen);

    req->payloadLen = sizeof(shm->payload);
    memcpy(req->payload, shm->payload, req->payloadLen);
}

int sendUdpRequest(int sockfd, struct sockaddr_in* srvAddr, const udp_req* req)
{
    int n = sendto(sockfd, req, sizeof(udp_req), 0,
                       (struct sockaddr*)srvAddr, sizeof(*srvAddr));
    if (n < 0) 
    { 
        perror("sendto"); 
        return -1; 
    }

    return 0;
}

int recvUdpReply(int sockfd, shm_msg* shm[], Process processes[])
{
    udp_rep reply;
    struct sockaddr_in src;
    socklen_t slen = sizeof(src);

    int n = recvfrom(sockfd, &reply, sizeof(reply), 0, (struct sockaddr*)&src, &slen);

    if(n <= 0) return 0;

    int idx = reply.owner - 1;

    printf("[Kernel] Reply recebida do servidor para owner=%d\n", reply.owner);

    shm[idx]->error = reply.error;
    shm[idx]->has_reply = 1;

    if (reply.payloadLen > 0) memcpy(shm[idx]->payload, reply.payload, reply.payloadLen);

    return 1;
}