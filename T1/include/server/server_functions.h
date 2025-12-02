#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include "udp_msg.h"

#include <string.h>
#include <stdio.h>

void ensureDirExists(const char* path);
void handleOperation(const udp_msg* req, udp_msg* rep);
void handleRead(const udp_msg* req, udp_msg* rep);
void handleWrite(const udp_msg* req, udp_msg* rep);
void handleCreateDir(const udp_msg* req, udp_msg* rep);
void handleRemoveDir(const udp_msg* req, udp_msg* rep);
void handleListDir(const udp_msg* req, udp_msg* rep);
long getFileSize(FILE* f);
void buildOwnerDir(const udp_msg* req);

#endif
