#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

void listar_diretorios(const char *caminho, int nivel) {
    DIR *dir;
    struct dirent *entrada;
    struct stat info;
    char caminho_completo[4096];

    dir = opendir(caminho);
    if (!dir) {
        printf("Erro ao abrir diretório '%s': %s\n", caminho, strerror(errno));
        return;
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
            printf("%*s[%s]\n", nivel * 2, "", entrada->d_name);
            listar_diretorios(caminho_completo, nivel + 1);
        } else if (S_ISREG(info.st_mode)) {
            printf("%*s%s (%lld bytes)\n", nivel * 2, "", entrada->d_name, (long long)info.st_size);
        }
    }

    closedir(dir);
}

int main(void) {
    printf("Estrutura de diretórios a partir do atual:\n");
    listar_diretorios(".", 0);
    return 0;
}
