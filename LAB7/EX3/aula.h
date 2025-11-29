#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SHM  101
#define SEM  202

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int setSemValue(int semId) {
    union semun semUnion;
    semUnion.val = 1;
    return semctl(semId, 0, SETVAL, semUnion);
}

void delSemValue(int semId) {
    union semun semUnion;
    semctl(semId, 0, IPC_RMID, semUnion);
}

int semaforoP(int semId) {
    struct sembuf semB;
    semB.sem_num = 0;
    semB.sem_op = -1; 
    semB.sem_flg = SEM_UNDO;
    semop(semId, &semB, 1);
    return 0;
}

int semaforoV(int semId) {
    struct sembuf semB;
    semB.sem_num = 0;
    semB.sem_op = 1; 
    semB.sem_flg = SEM_UNDO;
    semop(semId, &semB, 1);
    return 0;
}
