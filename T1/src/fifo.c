#include "fifo.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

int makeFIFO(const char* fifo_path) {
    if(mkfifo(fifo_path, 0666) == -1 && errno != EEXIST){
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
