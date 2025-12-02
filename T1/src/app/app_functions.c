#include "app/app_functions.h"

#include <sys/mman.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void print_syscall_info(int owner, int type, shm_msg *shm)
{
    if (type == 0 || type == 1) { // READ ou WRITE
        printf("[App %d] syscall: %s\n"
               "    Path: %s\n"
               "    Offset: %d\n"
               "    PayloadLen: %d\n",
               owner,
               (type == 0) ? "READ" : "WRITE",
               shm->path,
               shm->offset,
               shm->payloadLen);

        // Exibir payload apenas no WRITE e somente se houver dados
        if (type == 1 && shm->payloadLen > 0) {
            printf("    Payload: \"");
            for (int k = 0; k < shm->payloadLen; k++) {
                putchar(shm->payload[k]);
            }
            printf("\"\n");
        }

    } else if (type == 2 || type == 3) { // ADD_DIR ou REMOVE_DIR
        printf("[App %d] syscall: %s\n"
               "    Path: %s\n"
               "    DirName: %s\n",
               owner,
               (type == 2) ? "ADD_DIR" : "REMOVE_DIR",
               shm->path,
               shm->dirname);

    } else { // LISTDIR
        printf("[App %d] syscall: LISTDIR\n"
               "    Path: %s\n",
               owner,
               shm->path);
    }
}


int open_shared_memory(const char *name, shm_msg **shm) {
    int shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd < 0) {
        printf("Erro ao abrir a shared_memory\n");
        return -1;
    }

    *shm = mmap(NULL, sizeof(shm_msg),
                PROT_READ | PROT_WRITE,
                MAP_SHARED, shm_fd, 0);

    if (*shm == MAP_FAILED) {
        printf("Erro ao mapear a shared_memory\n");
        return -1;
    }
    return 0;
}

void prepare_syscall(shm_msg *shm, int owner, int offsets[]) {
    // memset(shm, 0, sizeof(shm_msg));
    shm->owner = owner;

    int type = rand() % 5;   // 0=read,1=write,2=add,3=rem,4=listdir
    int d = rand() % 3;
    int f = rand() % 5;

    shm->offset = offsets[rand() % 5];

    static const char* ops[] = {"RD", "WR", "DC", "DR", "DL"};
    strcpy(shm->op, ops[type]);

    // Escolhe aleatoriamente entre o diretório próprio e o A0
    int target_dir = (rand() % 2 == 0) ? owner : 0;
    int dir_num = 1 + rand() % 3;  // 1, 2 ou 3
    char dirname[16];
    sprintf(dirname, "novoDir%d", dir_num);

    // Monta path conforme tipo
    if (type == 0 || type == 1) { // READ ou WRITE
        sprintf(shm->path, "/A%d/file%d", target_dir, d);
    } else {                     // ADD, REM, LISTDIR
        sprintf(shm->path, "/A%d", target_dir);
    }

    if (type == 1) { // WRITE
        int empty_write = rand() % 5 == 0;
        if (empty_write) {
            shm->payloadLen = 0;
        } else {
            memset(shm->payload, 'A' + (owner % 26), 16);
            shm->payloadLen = 16;
        }
    }
    else if (type == 2) { // ADD DIR
        strcpy(shm->dirname, dirname);
        shm->strlenDirName = strlen(shm->dirname);
    } else if (type == 3) { // REMOVE DIR
        strcpy(shm->dirname, dirname);
        shm->strlenDirName = strlen(shm->dirname);
    }

    shm->strlenPath = strlen(shm->path);
    print_syscall_info(owner, type, shm);
    shm->has_reply = 0;
    shm->has_request = 1;
}


void handle_reply(shm_msg *shm, int owner) {
    if (!shm->has_reply)
        return;

    printf("[App %d] <- Resposta recebida para operação %s\n",
           owner, shm->op);

    if (shm->error < 0) {
        printf("   ERRO: código %d\n", shm->error);
        clear_shm_fields(shm);
        return;
    }

    if (strcmp(shm->op, "RD") == 0) {
        printf("   READ OK: payload recebido = \"");
        for (int i = 0; i < 16; i++)
            putchar(shm->payload[i]);
        printf("\"\n");

    } else if (strcmp(shm->op, "WR") == 0) {
        printf("   WRITE OK: 16 bytes foram escritos.\n");

    } else if (strcmp(shm->op, "DC") == 0) {
        printf("   ADD DIR OK: novo path = %s\n", shm->path);

    } else if (strcmp(shm->op, "DR") == 0) {
        printf("   REMOVE OK: novo path = %s\n", shm->path);

    } else if (strcmp(shm->op, "DL") == 0) {
        printf("   LISTDIR OK:\n");
        printf("   payload (compactado) = \"");
        for (int i = 0; i < 16; i++)
            putchar(shm->payload[i]);
        printf("\"\n");
        printf("   strlenPath = %d\n", shm->strlenPath);
        printf("   strlenDirName = %d\n", shm->strlenDirName);
    }

    clear_shm_fields(shm);
}

void clear_shm_fields(shm_msg *shm) {
    shm->has_reply = 0;
    shm->has_request = 0;
    shm->error = 0;

    memset(shm->op, 0, sizeof(shm->op));
    memset(shm->path, 0, sizeof(shm->path));
    memset(shm->dirname, 0, sizeof(shm->dirname));
    memset(shm->payload, 0, sizeof(shm->payload));

    shm->payloadLen = 0;

    shm->offset = 0;
    shm->strlenPath = 0;
    shm->strlenDirName = 0;
}