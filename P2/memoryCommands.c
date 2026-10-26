// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

#include "dynamicList.h"
#include "memoryCommands.h"

enum allocationType {
    MALLOC,
    SHARED,
    MAPPED
};

typedef struct memory {
    void *memoryAddress;
    size_t size;
    time_t allocationTime;
    enum allocationType type;

    union {
        struct {
            key_t key;
            int shmid;
        } shared;

        struct {
            int fd;
            char *fileName;
        } mapped;
    } u;
} memory;

static void LiberarMemoria(void *ptr) {
    memory *of = (memory *)ptr;

    if (of != NULL) {
        if (of->type == MAPPED && of->u.mapped.fileName != NULL) {
            free(of->u.mapped.fileName);
        }
        free(of);
    }
}

void VaciarMemoria(tList *MemoriaAlocada) {
    deleteListWithData(MemoriaAlocada, LiberarMemoria);
}

/* ───────────────────── COMANDOS DE MEMORIA ───────────────────── */

int Cmd_allocate(char *tr[], tList *MemoriaAlocada) {
}
void Cmd_deallocate() {
}
void Cmd_io() {
}
void Cmd_memory() {
}
void Cmd_read() {
}
void Cmd_write() {
}
void Cmd_recurse() {
}
void Cmd_memfill() {
}
void Cmd_memdump() {
}
