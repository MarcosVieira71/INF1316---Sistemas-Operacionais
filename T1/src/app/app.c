#include "fifo.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>

#include "shm_msg.h"
#include "app_functions.h"

#define MAX 20

int main(int argc, char *argv[]) {
    if (argc < 2) {

        printf("Uso: ./app <n>\n");
        exit(1);
    }

    int owner = atoi(argv[1]);
    printf("o que tá indo aqui ? %d\n", owner);
    int PC = 0;
    int pid = getpid();

    int offsets[] = {0,16,32,48,64};

    char shm_name[32];
    sprintf(shm_name, "/shm_A%d", owner);

    shm_msg *shm = NULL;

    if (open_shared_memory(shm_name, &shm) < 0)
        exit(1);

    printf("[App %d] iniciado como A%d (PID=%d)\n", owner, owner, pid);
    srand(pid ^ time(NULL));

    while (PC < MAX) {

        usleep(500000);
        PC++;

        int r = rand() % 100;

        if (r < 10) {
            prepare_syscall(shm, owner, offsets);
        }

        handle_reply(shm, owner);

        printf("[App %d] executando... PC=%d\n", owner, PC);
        usleep(500000);
    }

    printf("[App %d] finalizado. PC=%d\n", owner, PC);
    return 0;
}
