#ifndef FIFO_H
#define FIFO_H

#define FIFO_IRQ "/tmp/fifo_irq"
#define FIFO_SYSCALL "/tmp/fifo_syscall"

int makeFIFO(const char* fifo_path);
int openFIFO(int* fd, const char* fifo_path, int flags);

#endif
