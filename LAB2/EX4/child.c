#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <time.h>

#include "utils.h"
int main(int argc, char *argv[]) {
    int id = atoi(argv[1]);
    int* ptr = (int*)shmat(id, NULL, 0);
    errorShMat(ptr);

    srand(time(NULL) ^ getpid()); 
    int seq = 0;
    ptr[1] = seq;
    
    while(1) {
        sleep(rand() % 2 + 1);   

        int val = rand() % 20;  
        seq++;

        ptr[0] = val;
        ptr[1] = seq;   

        printf("Filho %d escreveu valor %d seq %d\n", getpid(), val, seq);
    }

    shmdt(ptr);
    return 0;
}
