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

/* ───────────────────── FUNCIONES AUXILIARES ───────────────────── */

int mallocLocal(ssize_t siz, tList *list) {
    if (siz <= 0) {
        return -1;
    }

    void *alocated = malloc(siz);
    memory *item = malloc(sizeof(struct memory));

    if (item == NULL || alocated == NULL) {
        free(alocated);
        free(item);
        return -1;
    }

    item->memoryAddress = alocated;
    item->allocationTime = time(NULL);
    item->size = siz;
    item->type = MALLOC;

    if (insertItem(item, LNULL, list)) {
        free(alocated);
        LiberarMemoria(item);
        return -1;
    }

    return 0;
}

int createSharedLocal(key_t cl, ssize_t siz, tList *list) {

    if (siz <= 0) {
        return -1;
    }

    int shmid = shmget(cl, siz, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        return -1;
    }
    void *address = shmat(shmid, NULL, 0);
    if (address == (void *)-1) {
        shmctl(shmid, IPC_RMID, NULL);
        return -1;
    }

    memory *item = malloc(sizeof(struct memory));
    if (item == NULL) {
        shmdt(address);
        shmctl(shmid, IPC_RMID, NULL);
        return -1;
    }

    item->memoryAddress = address;
    item->allocationTime = time(NULL);
    item->size = siz;
    item->type = SHARED;
    item->u.shared.key = cl;
    item->u.shared.shmid = shmid;

    if (insertItem(item, LNULL, list)) {
        shmdt(address);
        shmctl(shmid, IPC_RMID, NULL);
        LiberarMemoria(item);
        return -1;
    }

    return 0;
}

int sharedLocal(key_t cl, tList *list) {
    int shmid = shmget(cl, 0, 0666);
    if (shmid == -1) {
        return -1;
    }

    struct shmid_ds buf;
    if (shmctl(shmid, IPC_STAT, &buf) == -1) {
        return -1;
    }

    void *address = shmat(shmid, NULL, 0);
    if (address == (void *)-1) {
        return -1;
    }

    memory *item = malloc(sizeof(struct memory));
    if (item == NULL) {
        shmdt(address);
        return -1;
    }

    item->memoryAddress = address;
    item->allocationTime = time(NULL);
    item->size = buf.shm_segsz;
    item->type = SHARED;
    item->u.shared.key = cl;
    item->u.shared.shmid = shmid;

    if (insertItem(item, LNULL, list)) {
        shmdt(address);
        free(item);
        return -1;
    }

    return 0;
}

int mmapLocal(char *fil, char *perm, tList *list) {
    int prot = 0;

    if (strchr(perm, 'r')) prot |= PROT_READ;
    if (strchr(perm, 'w')) prot |= PROT_WRITE;
    if (strchr(perm, 'x')) prot |= PROT_EXEC;
    int oflags = strchr(perm, 'w') ? O_RDWR : O_RDONLY;

    int fd = open(fil, oflags);

    if (fd == -1) {
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1 || st.st_size <= 0) {
        close(fd);
        return -1;
    }

    void *addr = mmap(NULL, st.st_size, prot, MAP_SHARED, fd, 0);

    if (addr == MAP_FAILED) {
        close(fd);
        return -1;
    }

    memory *item = malloc(sizeof(struct memory));
    char *name = strdup(fil);
    if (item == NULL || name == NULL) {
        free(item);
        free(name);
        munmap(addr, st.st_size);
        close(fd);
        return -1;
    }

    item->memoryAddress = addr;
    item->allocationTime = time(NULL);
    item->size = st.st_size;
    item->type = MAPPED;
    item->u.mapped.fd = fd;
    item->u.mapped.fileName = name;

    if (insertItem(item, LNULL, list)) {
        LiberarMemoria(item);
        munmap(addr, st.st_size);
        close(fd);
        return -1;
    }

    return 0;
}

/* ───────────────────── COMANDOS DE MEMORIA ───────────────────── */

int Cmd_allocate(char *tr[], tList *list) {
    if (tr[0] == NULL) {
        fprintf(stderr, "allocate: missing option\n");
        return -1;
    }

    if (!strcmp(tr[0], "-malloc")) {
        ssize_t siz;

        if (tr[1] == NULL) {
            fprintf(stderr, "allocate: -malloc needs <siz>\n");
            return -1;
        }

        siz = strtol(tr[1], NULL, 10);

        if (siz <= 0) {
            fprintf(stderr, "allocate: -malloc needs a positive size\n");
            return -1;
        }

        if (mallocLocal(siz, list)) {
            fprintf(stderr, "allocate: \n");
            return -1;
        }

        return 0;

    } else if (!strcmp(tr[0], "-createshared")) {
        key_t cl;
        ssize_t siz;

        if (tr[1] == NULL || tr[2] == NULL) {
            fprintf(stderr, "allocate: -createshared needs <key> <size>\n");
            return -1;
        }

        cl = atoi(tr[1]);
        siz = strtol(tr[2], NULL, 10);

        if (siz <= 0) {
            fprintf(stderr, "allocate: size needs to be greater than 0\n");
            return -1;
        }

        if (createSharedLocal(cl, siz, list)) {
            fprintf(stderr, "allocate: \n");
            return -1;
        }

        return 0;

    } else if (!strcmp(tr[0], "-shared")) {
        key_t cl;

        if (tr[1] == NULL) {
            fprintf(stderr, "allocate: -shared needs a valid key\n");
            return -1;
        }

        cl = strtol(tr[1], NULL, 10);

        if (sharedLocal(cl, list)) {
            fprintf(stderr, "allocate: \n");
            return -1;
        }

        return 0;

    } else if (!strcmp(tr[0], "-mmap")) {
        if (tr[1] == NULL || tr[2] == NULL) {
            fprintf(stderr, "allocate: -mmap needs <file> <perm>\n");
            return -1;
        }

        if (mmapLocal(tr[1], tr[2], list)) {
            fprintf(stderr, "allocate: \n");
            return -1;
        }

        return 0;

    } else {
        fprintf(stderr, "allocate: unknown option %s\n", tr[0]);
        return -1;
    }
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
