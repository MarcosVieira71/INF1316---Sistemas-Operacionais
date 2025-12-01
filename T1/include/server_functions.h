#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include "udp_msg.h"

#include <string.h>


void handleOperation(const udp_msg* req, udp_msg* rep);
void handleRead(const udp_msg* req, udp_msg* rep);
void handleWrite(const udp_msg* req, udp_msg* rep);
void handleCreateDir(const udp_msg* req, udp_msg* rep);
void handleRemoveDir(const udp_msg* req, udp_msg* rep);
void handleListDir(const udp_msg* req, udp_msg* rep);
long getFileSize(FILE* f);

#endif
