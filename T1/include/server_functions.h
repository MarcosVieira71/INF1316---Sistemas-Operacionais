#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include "udp_rep.h"
#include "udp_req.h"

#include <string.h>


void handleOperation(const udp_req* req, udp_rep* rep);

void handleRead(const udp_req* req, udp_rep* rep);

#endif
