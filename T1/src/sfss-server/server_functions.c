#include "server/server_functions.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PAYLOAD 16
#define ROOT_DIR "SFS-root-dir"

void ensureDirExists(const char* path)
{
    if (mkdir(path, 0777) != 0)
    {
        if (errno != EEXIST) {
            perror("mkdir");
        }
    }
}

void handleOperation(const udp_msg* req, udp_msg* rep)
{
    memset(rep->payload, 0, MAX_PAYLOAD);
    rep->payloadLen = 0;

    buildOwnerDir(req);
    
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

void handleRead(const udp_msg* req, udp_msg* rep)
{
    char fullpath[256];
    sprintf(fullpath, "%s%s", ROOT_DIR, req->path);

    FILE* f = fopen(fullpath, "rb");
    if(!f)
    {
        rep->offset = -1;
        rep->payloadLen = 0;
        return;
    }

    long fileSize = getFileSize(f);

    // Retorna offset negativo caso o offset seja maior que o tamanho do arquivo
    if (req->offset >= fileSize)
    {
        rep->offset = -2; 
        rep->payloadLen = 0;
        fclose(f);
        return;
    }

    fseek(f, req->offset, SEEK_SET);

    int n = fread(rep->payload, 1, MAX_PAYLOAD, f);
    rep->payloadLen = n;
    rep->error = 0;
    rep->offset = n;

    fclose(f);
}

void handleWrite(const udp_msg* req, udp_msg* rep)
{
    char fullpath[256];
    sprintf(fullpath, "%s%s", ROOT_DIR, req->path);

    if (req->payloadLen == 0 && req->offset == 0)
    {
        if (unlink(fullpath) == 0)
            rep->offset = 0;
        else
            rep->offset = -1; 
        return;
    }

    // abre arquivo para leitura/escrita, cria se não existir
    FILE* f = fopen(fullpath, "r+b");
    if (!f)
    {
        f = fopen(fullpath, "w+b");  
        if (!f)
        {
            rep->offset = -2; 
            return;
        }
    }

    long fileSize = getFileSize(f);

    // Preenche espaços vazios com whitespace, caso offset seja maior que o tamanho do arquivo
    if(req->offset > fileSize)
    {
        long gap = req->offset - fileSize;
        fseek(f, fileSize, SEEK_SET);

        for(long i = 0; i < gap; i++)
        {
            fputc(0x20, f);
        }
    }
    
    fseek(f, req->offset, SEEK_SET);
    int n = fwrite(req->payload, 1, req->payloadLen, f);

    if (n != req->payloadLen)
        rep->offset = -3;  
    else
        rep->offset = req->offset;

    fclose(f);

}

void handleCreateDir(const udp_msg* req, udp_msg* rep)
{
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s%s/%s", ROOT_DIR, req->path, req->dirname);

    if (mkdir(fullpath, 0777) == 0)
    {
        rep->error = 0;
        snprintf(rep->path, sizeof(rep->path), "%s/%s", req->path, req->dirname);
        rep->pathLen = strlen(rep->path);
    }
    else {
        rep->error = -1;  
    }       
}

void handleRemoveDir(const udp_msg* req, udp_msg* rep)
{
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s/%s/%s", ROOT_DIR, req->path, req->dirname);

    if (rmdir(fullpath) == 0)
    {
        rep->error = 0;
        snprintf(rep->path, sizeof(rep->path), "%s", req->path);
        rep->pathLen = strlen(rep->path);
    }
    else rep->error = -1; 
}

void handleListDir(const udp_msg* req, udp_msg* rep)
{
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", ROOT_DIR, req->path);
    memset(&rep->listinfo, 0, sizeof(rep->listinfo));

    DIR* d = opendir(fullpath);
    if (!d) {
        rep->error = -1;
        return;
    }

    rep->listinfo.nrnames = 0;
    int pos = 0;

    struct dirent* ent;
    while ((ent = readdir(d)) != NULL)
    {
        // ignora . and ..
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        int nameLen = strlen(ent->d_name);
        int idx = rep->listinfo.nrnames;

        if (pos + nameLen >= MAX_ALLNAMES)
            break;

        rep->listinfo.fstlstpositions[idx][0] = pos;

        memcpy(rep->listinfo.allnames + pos, ent->d_name, nameLen);

        pos += nameLen;

        rep->listinfo.fstlstpositions[idx][1] = pos;

        rep->listinfo.isDir[idx] =
            (ent->d_type == DT_DIR ? 1 : 0);

        rep->listinfo.nrnames++;
    }

    closedir(d);

    rep->error = 0;
}


long getFileSize(FILE* f) 
{
    fseek(f, 0, SEEK_END);
    return ftell(f);
}

void buildOwnerDir(const udp_msg* req)
{
    char ownerDir[256];
    int id = -1;

    if(req->path[0] == '/' && req->path[1] == 'A') 
    {
        id = atoi(&req->path[2]);
        
    }
    if(id < 0) id = req->owner;

    snprintf(ownerDir, sizeof(ownerDir), "%s/A%d", ROOT_DIR, id);
    ensureDirExists(ownerDir);
}
