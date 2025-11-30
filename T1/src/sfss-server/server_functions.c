#include "server_functions.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PAYLOAD 256
#define ROOT_DIR "SFS-root-dir"

void handleOperation(const udp_req* req, udp_rep* rep)
{
    memset(rep->payload, 0, MAX_PAYLOAD);
    rep->payloadLen = 0;
    
    if (strcmp(req->op, "RD") == 0)
        handleRead(req, rep);

    else if (strcmp(req->op, "WR") == 0)
        handleWrite(req, rep);

    else if (strcmp(req->op, "DC") == 0)
        handleCreateDir(req, rep);

    else if (strcmp(req->op, "DR") == 0)
        handleRemoveDir(req, rep);

    else if (strcmp(req->op, "DL") == 0)
        handleListDir(req, rep);

    else {
        rep->error = -3; // Operação não reconhecida 
    }
}

void handleRead(const udp_req* req, udp_rep* rep)
{
    char fullpath[256];
    sprintf(fullpath, "%s/%s", ROOT_DIR, req->path);

    FILE* f = fopen(fullpath, "rb");
    if(!f)
    {
        rep->error = -1;
        return;
    }

    fseek(f, req->offset, SEEK_SET);

    int n = fread(rep->payload, 1, MAX_PAYLOAD, f);
    rep->payloadLen = n;
    rep->error = 0;

    fclose(f);
}

void handleWrite(const udp_req* req, udp_rep* rep)
{
    char fullpath[256];
    sprintf(fullpath, "%s/%s", ROOT_DIR, req->path);

    if (req->payloadLen == 0 && req->offset == 0)
    {
        if (unlink(fullpath) == 0)
            rep->error = 0;
        else
            rep->error = -1; 
        return;
    }

    // abre arquivo para leitura/escrita, cria se não existir
    FILE* f = fopen(fullpath, "r+b");
    if (!f)
    {
        f = fopen(fullpath, "w+b");  
        if (!f)
        {
            rep->error = -2; 
            return;
        }
    }

    // posiciona no offset
    fseek(f, req->offset, SEEK_SET);

    
    int n = fwrite(req->payload, 1, req->payloadLen, f);
    if (n != req->payloadLen)
        rep->error = -3;  
    else
        rep->error = 0;

    fclose(f);

}

void handleCreateDir(const udp_req* req, udp_rep* rep)
{
    char fullpath[256];
    sprintf(fullpath, "%s/%s/%s", ROOT_DIR, req->path, req->dirname);

    if (mkdir(fullpath, 0777) == 0)
        rep->error = 0;
    else
        rep->error = -1;  
}

void handleRemoveDir(const udp_req* req, udp_rep* rep)
{
    char fullpath[256];
    sprintf(fullpath, "%s/%s", ROOT_DIR, req->path);

    if (rmdir(fullpath) == 0)
        rep->error = 0;
    else
        rep->error = -1; 
}

void handleListDir(const udp_req* req, udp_rep* rep)
{
    char fullpath[256];
    sprintf(fullpath, "%s/%s", ROOT_DIR, req->path);

    DIR* d = opendir(fullpath);
    if (!d)
    {
        rep->error = -1; 
        return;
    }

    struct dirent* ent;
    int total = 0;

    while ((ent = readdir(d)) != NULL)
    {
        // ignorar "." e ".."
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        int len = strlen(ent->d_name);

        if (total + len + 1 >= MAX_PAYLOAD)
            break; // payload cheio

        memcpy(rep->payload + total, ent->d_name, len);
        total += len;

        rep->payload[total++] = '\n';
    }

    rep->payloadLen = total;
    rep->error = 0;

    closedir(d);
}
