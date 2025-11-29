#include "aula.h"

int main() {
    int shmId = shmget(KEY_SHM, sizeof(int), 0666);
    int semId = semget(KEY_SEM, 1, 0666);
    if (shmId >= 0) shmctl(shmId, IPC_RMID, NULL);
    if (semId >= 0) delSemValue(semId);
    printf("Recursos removidos.\n");
    return 0;
}
