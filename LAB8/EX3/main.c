#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

long long soma_tamanhos(const char *caminho) {
    DIR *dir;
    struct dirent *entrada;
    struct stat info;
    char caminho_completo[4096];
    long long soma = 0;

    dir = opendir(caminho);
    if (!dir) {
        fprintf(stderr, "Erro ao abrir diretório '%s': %s\n", caminho, strerror(errno));
        return 0;
    }

    while ((entrada = readdir(dir)) != NULL) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0)
            continue;

        snprintf(caminho_completo, sizeof(caminho_completo), "%s/%s", caminho, entrada->d_name);

        if (stat(caminho_completo, &info) == -1) {
            printf("Erro ao obter informações de '%s': %s\n", caminho_completo, strerror(errno));
            continue;
        }

        if (S_ISDIR(info.st_mode)) {
            soma += soma_tamanhos(caminho_completo);
        } else if (S_ISREG(info.st_mode)) {
            soma += info.st_size;
        }
    }

    closedir(dir);
    return soma;
}

int main(void) {
    const char *diretorio_inicial = ".";
    long long total = soma_tamanhos(diretorio_inicial);
    printf("Tamanho total dos arquivos: %lld bytes\n", total);
    return 0;
}
