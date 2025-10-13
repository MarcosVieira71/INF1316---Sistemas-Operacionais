

int enqueue(pid_t* fila, int* n, pid_t pid) {
    fila[*n] = pid;
    (*n)++;
    return *n;
}

pid_t dequeue(pid_t* fila, int* n) {
    if(*n == 0) return -1;
    
    pid_t pid = fila[0];
    for(int i=0;i<(*n)-1;i++){
        fila[i] = fila[i+1];
    }
    (*n)--;
    return pid;
}