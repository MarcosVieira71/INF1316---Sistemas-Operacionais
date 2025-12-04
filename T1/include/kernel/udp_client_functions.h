#ifndef UDP_CLIENT_FUNCTIONS_H
#define UDP_CLIENT_FUNCTIONS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "udp_msg.h"
#include "process.h"

#define NUM_PROC 5
#define MAX_WRITTEN_FILES 128
#define MAX_PATH_LEN 256

int createUdpSocket(const char* serverIp, int serverPort, struct sockaddr_in* srvAddr);
void buildReqFromShm(udp_msg* req, const shm_msg* shm); 
int sendUdpRequest(int sockfd, struct sockaddr_in* srvAddr, const udp_msg* req);
kernel_reply recvUdpReply(int sockfd, shm_msg* shm[], Process processes[], char written_files[NUM_PROC][MAX_WRITTEN_FILES][MAX_PATH_LEN], int written_files_count[NUM_PROC]);
 

#endif 