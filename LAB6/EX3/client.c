#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define FIFO_CLIENTE "fifoClienteParaServidor"
#define FIFO_SERVIDOR "fifoServidorParaCliente"
#define TAM_MAX 256

int main() {
    char buffer[TAM_MAX];

    while (1) {
        printf("Digite uma mensagem (ou 'sair' para encerrar): ");
        if (!fgets(buffer, TAM_MAX, stdin)) break;

        buffer[strcspn(buffer, "\n")] = '\0'; 

        if (strcmp(buffer, "sair") == 0) break;

        int fdEnvio = open(FIFO_CLIENTE, O_WRONLY);
        if (fdEnvio < 0) {
            perror("Erro ao abrir FIFO de envio");
            continue;
        }
        write(fdEnvio, buffer, strlen(buffer) + 1);
        close(fdEnvio);

        int fdRecebe = open(FIFO_SERVIDOR, O_RDONLY);
        if (fdRecebe < 0) {
            perror("Erro ao abrir FIFO de resposta");
            continue;
        }
        
        int lidos = read(fdRecebe, buffer, TAM_MAX - 1);
        close(fdRecebe);

        if (lidos > 0) {
            buffer[lidos] = '\0';
            printf("Resposta do servidor: %s\n", buffer);
        }
    }

    printf("Cliente encerrado.\n");
    return 0;
}
