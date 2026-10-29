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
    if (tr[0] == NULL) {
        fprintf(stderr, "allocate: missing option\n");
        return -1;
    }

    if (!strcmp(tr[0], "-malloc")) {
        if (tr[1] == NULL) {
            perror("allocate");
            return -1;
        }

        ssize_t siz = strtol(tr[1], NULL, 10);

        if (siz <= 0) {
            perror("allocate");
            return -1;
        }

        void *alocated = malloc(siz);
        memory *item = malloc(sizeof(struct memory));

        if (item == NULL || alocated == NULL) {
            perror("allocate");
            return -1;
        }

        item->memoryAddress = alocated;
        item->allocationTime = time(NULL);
        item->size = siz;
        item->type = MALLOC;

        if (!insertItem(item, LNULL, MemoriaAlocada)) {
            perror("allocate");
            free(alocated);
            LiberarMemoria(item);
            return -1;
        }

        return 0;

    } else if (!strcmp(tr[0], "-shared")) {
        if (tr[1] == NULL) {
            fprintf(stderr, "allocate: missing key\n");
            return -1;
        }

        key_t cl = strtol(tr[1], NULL, 10);

        int shmid = shmget(cl, 0, 0666);
        if (shmid == -1) {
            perror("allocate");
            return -1;
        }

        struct shmid_ds buf;
        if (shmctl(shmid, IPC_STAT, &buf) == -1) {
            perror("allocate");
            return -1;
        }

        void *address = shmat(shmid, NULL, 0);
        if (address == (void *)-1) {
            perror("allocate");
            return -1;
        }

        memory *item = malloc(sizeof(struct memory));
        if (item == NULL) {
            perror("allocate");
            shmdt(address); /* do NOT IPC_RMID: not ours */
            return -1;
        }

        item->memoryAddress = address;
        item->allocationTime = time(NULL);
        item->size = buf.shm_segsz;
        item->type = SHARED;
        item->u.shared.key = cl;
        item->u.shared.shmid = shmid;

        if (!insertItem(item, LNULL, MemoriaAlocada)) {
            fprintf(stderr, "allocate: could not insert block into list\n");
            shmdt(address);
            free(item);
            return -1;
        }
        return 0;

    } else if (!strcmp(tr[0], "-createshared")) {
        if (tr[1] == NULL || tr[2] == NULL) {
            perror("allocate");
            return -1;
        }

        key_t cl = atoi(tr[1]);
        ssize_t siz = strtol(tr[2], NULL, 10);

        if (siz <= 0) {
            perror("allocate");
            return -1;
        }

        int shmid = shmget(cl, siz, IPC_CREAT | IPC_EXCL | 0666);
        if (shmid == -1) {
            perror("allocate");
            return -1;
        }
        void *address = shmat(shmid, NULL, 0);
        if (address == (void *)-1) {
            perror("allocate");
            shmctl(shmid, IPC_RMID, NULL);
            return -1;
        }

        memory *item = malloc(sizeof(struct memory));
        if (item == NULL) {
            perror("allocate");
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

        if (!insertItem(item, LNULL, MemoriaAlocada)) {
            perror("allocate");
            shmdt(address);
            shmctl(shmid, IPC_RMID, NULL);
            LiberarMemoria(item);
            return -1;
        }

        return 0;

    } else if (!strcmp(tr[0], "-mmap")) {
        if (tr[1] == NULL || tr[2] == NULL) {
            perror("allocate");
            return -1;
        }

        char *fil = tr[1];
        char *perm = tr[2];

        int prot = 0;

        if (strchr(perm, 'r')) prot |= PROT_READ;
        if (strchr(perm, 'w')) prot |= PROT_WRITE;
        if (strchr(perm, 'x')) prot |= PROT_EXEC;
        int oflags = strchr(perm, 'w') ? O_RDWR : O_RDONLY;

        int fd = open(fil, oflags);

        if (fd == -1) {
            perror("allocate");
            return -1;
        }

        struct stat st;
        if (fstat(fd, &st) == -1 || st.st_size <= 0) {
            perror("allocate");
            close(fd);
            return -1;
        }

        void *addr = mmap(NULL, st.st_size, prot, MAP_SHARED, fd, 0);

        if (addr == MAP_FAILED) {
            perror("mmap");
            return -1;
        }

        memory *item = malloc(sizeof(struct memory));
        char *name = strdup(fil);
        if (item == NULL || name == NULL) {
            perror("allocate");
            free(name);
            LiberarMemoria(item);
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

        if (!insertItem(item, LNULL, MemoriaAlocada)) {
            perror("allocate");
            free(name);
            LiberarMemoria(item);
            munmap(addr, st.st_size);
            close(fd);
            return -1;
        }

        return 0;
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
