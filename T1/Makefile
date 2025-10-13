# Makefile para KernelSim

CC = gcc
CFLAGS = -Wall -g -std=c99

APP_EXEC = app
KERNEL_EXEC = kernel
INTER_EXEC = intercontroller

APP_SRC = app.c
KERNEL_SRC = kernel.c
INTER_SRC = intercontroller.c

all: $(APP_EXEC) $(KERNEL_EXEC) $(INTER_EXEC)

$(APP_EXEC): $(APP_SRC)
	$(CC) $(CFLAGS) -o $@ $<

$(KERNEL_EXEC): $(KERNEL_SRC)
	$(CC) $(CFLAGS) -o $@ $<

$(INTER_EXEC): $(INTER_SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(APP_EXEC) $(KERNEL_EXEC) $(INTER_EXEC) *.o

.PHONY: all clean
