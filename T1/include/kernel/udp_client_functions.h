#ifndef UDP_CLIENT_FUNCTIONS_H
#define UDP_CLIENT_FUNCTIONS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "udp_req.h"
#include "process.h"

int createUdpSocket(const char* serverIp, int serverPort, struct sockaddr_in* srvAddr);
void buildReqFromShm(udp_req* req, const shm_msg* shm); 
int sendUdpRequest(int sockfd, struct sockaddr_in* srvAddr, const udp_req* req);
kernel_reply recvUdpReply(int sockfd, shm_msg* shm[], Process processes[]);
 

#endif 