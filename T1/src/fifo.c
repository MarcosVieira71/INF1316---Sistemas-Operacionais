#include "fifo.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

int makeFIFO(const char* fifo_path) {
    unlink(fifo_path);

    if(mkfifo(fifo_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1 && errno != EEXIST){
        return -1;
    }
    return 0;
}

int openFIFO(int* fd, const char* fifo_path, int flags) {
    *fd = open(fifo_path, flags);
    if(*fd == -1){
        return -1;
    }
    return 0;
}
