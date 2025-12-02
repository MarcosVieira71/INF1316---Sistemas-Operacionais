#ifndef APP_FUNCTIONS_H
#define APP_FUNCTIONS_H

#include "shm_msg.h"

int open_shared_memory(const char *name, shm_msg **shm);
void prepare_syscall(shm_msg *shm, int owner, int offsets[]);
void handle_reply(shm_msg *shm, int owner);
void clear_shm_fields(shm_msg *shm);
void print_syscall_info(int owner, int type, shm_msg *shm);

#endif