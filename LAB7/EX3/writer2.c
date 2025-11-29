#include "aula.h"

int main() {
    int idMem, idSemaforo;
    int *valor;

    idMem = shmget(SHM, 4, 0666 | IPC_CREAT);
 
    valor = (int*) shmat(idMem, NULL, 0);

    idSemaforo = semget(SEM, 1, 0666 | IPC_CREAT);

    for (int i = 0; i < 5; i++) {
        semaforoP(idSemaforo);
        (*valor) += 5;
        printf("Valor atual = %d, soma +5\n", *valor);
        semaforoV(idSemaforo);
        sleep(1);
    }

    shmdt(valor);
    return 0;
}
