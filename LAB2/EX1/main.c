#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

void printMatrix(int* m, int r, int c)
{
    printf("Matriz: \n");
    for(int i = 0; i < r; i++){
        for(int j = 0; j < c; j++)
        {
            printf(" %d ", m[i * c + j]);
        }
        printf("\n");
    }
}

void fillMatrix(int * m, int r, int c)
{
    for(int i = 0; i < r; i++){
        for(int j = 0; j < c; j++){
            m[i * c + j] = rand() % 10;
        }
    }
    
}

int main(void)
{
    int rows = 3;
    int cols = 3;
    
    int shms[3];
    int* shAtt[3];
    for(int i = 0; i < 3; i++){
        shms[i] = shmget (IPC_PRIVATE, rows * cols * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
        errorShGet(shms[i]);
        shAtt[i] = (int*)shmat(shms[i], NULL, 0);
        errorShMat(shAtt[i]);
    }
        
    pid_t pids[rows];


    fillMatrix(shAtt[0], rows, cols);
    fillMatrix(shAtt[1], rows, cols);

    printMatrix(shAtt[0], rows, cols);
    printf("*************\n");
    printMatrix(shAtt[1], rows, cols);
    
    for(int i = 0; i < rows; i++)
    {
        pid_t p = fork();
        if(p == 0)
        {
            for(int j = 0; j < cols; j++)
            {
                shAtt[2][i * cols + j] = shAtt[0][i * cols + j] + shAtt[1][i * cols + j];
            }

            for(int k = 0; k < 3; k++){
                shmdt(shAtt[k]);
            }
            exit(0);
        }
        else{
            pids[i] = p;
        }
    }

    for (int i = 0; i < rows; i++) {
        waitpid(pids[i], NULL, 0);
    }
    printf("*************\n");

    printf("Matriz Soma:\n");
    printMatrix(shAtt[2], rows, cols);

    for(int i = 0; i < 3; i++){
        shmdt(shAtt[i]);
        shmctl(shms[i], IPC_RMID, NULL);
    }

    return 0;
}