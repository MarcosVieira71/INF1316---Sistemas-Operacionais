#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

time_t inicio = 0;

void inicio_ligacao(int sig) {
    inicio = time(NULL); 
    printf("Ligação iniciada em %s", ctime(&inicio));
}

void fim_ligacao(int sig) {
    if (inicio == 0) {
        printf("Não há ligação em andamento\n");
        return;
    }

    time_t fim = time(NULL);
    int duracao = (int)difftime(fim, inicio); 

    float preco = 0.0;
    if (duracao <= 60) {
        preco = duracao * 0.02;
    } else {
        preco = (60 * 0.02) + ((duracao - 60) * 0.01);
    }

    printf("Ligação encerrada em %s\n", ctime(&fim));
    printf("Preço: R$ %.2f\n", preco);

    exit(0);
}

int main() {
    signal(SIGUSR1, inicio_ligacao);
    signal(SIGUSR2, fim_ligacao);

    printf("Monitor de chamadas iniciado (PROCESSO = %d).\n", getpid());
    printf("SIGUSR1 para iniciar ligação | SIGUSR2 para encerrar.\n");

    while (1) {
        pause(); 
    }

    return 0;
}
