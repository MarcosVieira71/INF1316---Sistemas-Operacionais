#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

void handleError(int sig) {
    printf("Div por 0.\n");
    exit(1);
}

int main() {
    int a, b;

    signal(SIGFPE, handleError);

    printf("Digite o primeiro número: ");
    scanf("%d", &a);

    printf("Digite o segundo número: ");
    scanf("%d", &b);

    printf("\nResultados:\n");
    printf("Soma: %d\n", a + b);
    printf("Subtração: %d\n", a - b);
    printf("Multiplicação: %d\n", a * b);
    printf("Divisão: %d\n", a / b);
 

    return 0;
}
