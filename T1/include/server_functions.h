#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include "udp_rep.h"
#include "udp_req.h"

#include <string.h>


void handleOperation(const udp_req* req, udp_rep* rep);
void handleRead(const udp_req* req, udp_rep* rep);
void handleWrite(const udp_req* req, udp_rep* rep);
void handleCreateDir(const udp_req* req, udp_rep* rep);
void handleRemoveDir(const udp_req* req, udp_rep* rep);
void handleListDir(const udp_req* req, udp_rep* rep);
long getFileSize(FILE* f);

#endif
