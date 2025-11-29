#include "aula.h"

int main() {
    int idMem, idSemaforo;
    int *valor;

    idMem = shmget(SHM, 4, 0666 | IPC_CREAT);

    valor = (int *) shmat(idMem, NULL, 0);

    idSemaforo = semget(SEM, 1, 0666 | IPC_CREAT);

    setSemValue(idSemaforo); 

    for (int i = 0; i < 5; i++) {
        sleep(1);
        semaforoP(idSemaforo);
        (*valor) += 1;
        printf("Valor atual = %d soma +1\n", *valor);
        semaforoV(idSemaforo);
    }

    shmdt(valor);
    return 0;
}
