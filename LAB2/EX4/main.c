#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

int main(void)
{
    int shms[2];
    int* shAtt[2];
    int pids[2];
    for(int i = 0; i < 2; i++){
        shms[i] = shmget (IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
        errorShGet(shms[i]);
        shAtt[i] = (int*)shmat(shms[i], NULL, 0);
        errorShMat(shAtt[i]);
    }

    for(int i = 0; i < 2; i++) {
        pids[i] = fork();
        if(pids[i] == 0) {
            char arg[8];             
            sprintf(arg, "%d", shms[i]);
            execl("./child", "child", arg, NULL);
            printf("Exec falhou\n");
            exit(1);
        }
    }
    
    int lastChild1 = 0;
    int lastChild2 = 0;
    int n = 0;
    
    while(n < 5) {
        if(shAtt[0][1] != lastChild1 && shAtt[1][1] != lastChild2) {
            printf("Produto: %d\n", shAtt[0][0] * shAtt[1][0]);
            lastChild1 = shAtt[0][1];
            lastChild2 = shAtt[1][1];
            n++;
        }
    }
    for(int i = 0; i < 2; i++) {
        shmdt(shAtt[i]);
        kill(pids[i], SIGTERM);     
        shmctl(shms[i], IPC_RMID, NULL);
    }
    return 0;
}