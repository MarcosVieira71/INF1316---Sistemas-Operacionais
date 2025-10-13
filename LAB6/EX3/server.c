#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define FIFO_CLIENTE "fifoClienteParaServidor"
#define FIFO_SERVIDOR "fifoServidorParaCliente"
#define TAM 256

void toUppercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

int main(void) {
    char buffer[TAM];

    if (access(FIFO_CLIENTE, F_OK) == -1) {
        if (mkfifo(FIFO_CLIENTE, 0666) == -1) {
            perror("Erro ao criar FIFO do cliente");
            exit(1);
        }
    }
    if (access(FIFO_SERVIDOR, F_OK) == -1) {
        if (mkfifo(FIFO_SERVIDOR, 0666) == -1) {
            perror("Erro ao criar FIFO do servidor");
            exit(1);
        }
    }

    printf("[Servidor] Rodando... Aguardando mensagens\n");

    int fdLeitura = open(FIFO_CLIENTE, O_RDONLY);
    if (fdLeitura < 0) {
        perror("Erro ao abrir FIFO de leitura");
        exit(1);
    }
    int fdEscrita = open(FIFO_SERVIDOR, O_WRONLY);
    if (fdEscrita < 0) {
        perror("Erro ao abrir FIFO de escrita");
        exit(1);
    }

    while (1) {
        int lidos = read(fdLeitura, buffer, TAM - 1);
        if (lidos <= 0) continue;

        buffer[lidos] = '\0';
        printf("[Servidor] Recebido: %s\n", buffer);

        toUppercase(buffer);

        write(fdEscrita, buffer, strlen(buffer) + 1);
    }

    close(fdLeitura);
    close(fdEscrita);
    return 0;
}
