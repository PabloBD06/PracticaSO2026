// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
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

#define BYTES_PER_LINE 10

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

void ListarMemoria(tList *MemoriaAlocada) {
    printf("******Lista de bloques asignados\n");

    for (tPosL pos = first(*MemoriaAlocada); pos != LNULL; pos = next(pos, *MemoriaAlocada)) {

        memory *m = (memory *)getItem(pos, *MemoriaAlocada);

        char date[7];
        char hour[6];

        struct tm *tm_info = localtime(&m->allocationTime);

        if (tm_info == NULL) {
            continue;
        }

        strftime(date, sizeof(date), "%b %d", tm_info);
        strftime(hour, sizeof(hour), "%H:%M", tm_info);

        printf("    %p %15zu %s %s ",
               m->memoryAddress, m->size, date, hour);

        if (m->type == MALLOC) {
            printf("malloc\n");
        } else if (m->type == SHARED) {
            printf("shared (key %d)\n",
                   (int)m->u.shared.key);
        } else if (m->type == MAPPED) {
            printf("%s (descriptor %d)\n",
                   m->u.mapped.fileName,
                   m->u.mapped.fd);
        }
    }
}

/* ───────────────────── FUNCIONES AUXILIARES ───────────────────── */

int mallocLocal(ssize_t siz, tList *list) {
    if (siz <= 0) {
        return -1;
    }

    void *addr = malloc(siz);
    memory *item = malloc(sizeof(struct memory));

    if (item == NULL || addr == NULL) {
        free(addr);
        free(item);
        return -1;
    }

    item->memoryAddress = addr;
    item->allocationTime = time(NULL);
    item->size = siz;
    item->type = MALLOC;

    if (!insertItem(item, LNULL, list)) {
        free(addr);
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
    void *addr = shmat(shmid, NULL, 0);
    if (addr == (void *)-1) {
        shmctl(shmid, IPC_RMID, NULL);
        return -1;
    }

    memory *item = malloc(sizeof(struct memory));
    if (item == NULL) {
        shmdt(addr);
        shmctl(shmid, IPC_RMID, NULL);
        return -1;
    }

    item->memoryAddress = addr;
    item->allocationTime = time(NULL);
    item->size = siz;
    item->type = SHARED;
    item->u.shared.key = cl;
    item->u.shared.shmid = shmid;

    if (!insertItem(item, LNULL, list)) {
        shmdt(addr);
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

    void *addr = shmat(shmid, NULL, 0);
    if (addr == (void *)-1) {
        return -1;
    }

    memory *item = malloc(sizeof(struct memory));
    if (item == NULL) {
        shmdt(addr);
        return -1;
    }

    item->memoryAddress = addr;
    item->allocationTime = time(NULL);
    item->size = buf.shm_segsz;
    item->type = SHARED;
    item->u.shared.key = cl;
    item->u.shared.shmid = shmid;

    if (!insertItem(item, LNULL, list)) {
        shmdt(addr);
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
    // Tendremos que usar cmd open para que quede reflejado en nuestra lista
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

    if (!insertItem(item, LNULL, list)) {
        LiberarMemoria(item);
        munmap(addr, st.st_size);
        close(fd);
        return -1;
    }

    return 0;
}

// Creo que puede desbordar si size es muy grande
void doDeallocateMalloc(size_t size, tList *MemoriaAlocada) {
    for (tPosL pos = first(*MemoriaAlocada); pos != LNULL; pos = next(pos, *MemoriaAlocada)) {
        tItemL item = getItem(pos, *MemoriaAlocada);
        memory *m = (memory *)item;
        if (m->type == MALLOC && m->size == size) {
            free(m->memoryAddress);
            LiberarMemoria(m);
            deleteAtPosition(pos, MemoriaAlocada);
            return;
        }
    }
    printf("No hay bloque de ese tamano asignado con malloc");
}
void doDeallocateShared(key_t key, tList *MemoriaAlocada) {
    for (tPosL pos = first(*MemoriaAlocada); pos != LNULL; pos = next(pos, *MemoriaAlocada)) {
        tItemL item = getItem(pos, *MemoriaAlocada);
        memory *m = (memory *)item;
        if (m->type == SHARED && m->u.shared.key == key) {
            if (shmdt(m->memoryAddress) == -1) {
                perror("shmdt");
                return;
            }
            LiberarMemoria(m);
            deleteAtPosition(pos, MemoriaAlocada);
            return;
        }
    }
    printf("No hay bloque de esa clave mapeado para el proceso");
}
void doDeallocateDelkey(key_t key) {
    int shmid = shmget(key, 0, 0666);

    if (shmid == -1) {
        perror("shmget");
        return;
    }
    // Me falta entender que son el segundo y tercer argumento
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        return;
    }
}

void doDeallocateMmap(char *filename, tList *MemoriaAlocada) {
    for (tPosL pos = first(*MemoriaAlocada); pos != LNULL; pos = next(pos, *MemoriaAlocada)) {
        tItemL item = getItem(pos, *MemoriaAlocada);
        memory *m = (memory *)item;
        if (m->type == MAPPED && strcmp(m->u.mapped.fileName, filename) == 0) {
            if (munmap(m->memoryAddress, m->size) == -1) {
                perror("munmap");
                return;
            }
            // tengo que buscar una forma de llamar a cmd close porque si no no usa nuestra lista de ficheros
            close(m->u.mapped.fd);
            LiberarMemoria(m);
            deleteAtPosition(pos, MemoriaAlocada);
            return;
        }
    }
    printf("No hay bloque de esa clave mapeado para el proceso");
}

void doDeallocateAddress(void *addr, tList *MemoriaAlocada) {
    for (tPosL pos = first(*MemoriaAlocada); pos != LNULL; pos = next(pos, *MemoriaAlocada)) {
        tItemL item = getItem(pos, *MemoriaAlocada);
        memory *m = (memory *)item;
        if (m->memoryAddress == addr) {
            if (m->type == MALLOC) {
                free(m->memoryAddress);
                LiberarMemoria(m);
                deleteAtPosition(pos, MemoriaAlocada);
                return;
            } else if (m->type == SHARED) {
                if (shmdt(m->memoryAddress) == -1) {
                    perror("shmdt");
                    return;
                }
                LiberarMemoria(m);
                deleteAtPosition(pos, MemoriaAlocada);
                return;
            } else if (m->type == MAPPED) {
                if (munmap(m->memoryAddress, m->size) == -1) {
                    perror("munmap");
                    return;
                }
                close(m->u.mapped.fd);
                LiberarMemoria(m);
                deleteAtPosition(pos, MemoriaAlocada);
                return;
            }
        }
    }
    printf("No ejecutado: No such file or directory");
}

static void printAddress(void *addr) {
    printf("%p-> ", addr);
}

void dumpMemory(void *addr, int siz) {
    unsigned char *p = addr;

    for (int offset = 0; offset < siz; offset += BYTES_PER_LINE) {

        int n = siz - offset;
        if (n > BYTES_PER_LINE) n = BYTES_PER_LINE;

        printAddress(p + offset);
        for (int i = 0; i < n; i++) {
            putchar(isprint(p[offset + i]) ? p[offset + i] : ' ');
        }
        printf("\n");

        printAddress(p + offset);
        for (int i = 0; i < n; i++) {
            printf("%02x ", p[offset + i]);
        }
        printf("\n");
    }
}

/* ───────────────────── COMANDOS DE MEMORIA ───────────────────── */

int Cmd_allocate(char *tr[], tList *list) {
    if (tr[0] == NULL) {
        ListarMemoria(list);
        return 0;
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

void Cmd_deallocate(char *tr[], tList *MemoriaAlocada) {
    // Sin argumentos-> Listar memoria
    if (tr[0] == NULL) {
        ListarMemoria(MemoriaAlocada);
        return;
    }
    // Caso malloc
    if (strcmp(tr[0], "-malloc") == 0) {
        if (tr[1] == NULL) {
            printf("Faltan argumentos\n");
            return;
        }

        int size = atoi(tr[1]);

        if (size == 0) {
            printf("No se asignan bloques de 0 bytes\n");
            return;
        }

        doDeallocateMalloc((size_t)size, MemoriaAlocada);
    }
    // Caso shared
    else if (strcmp(tr[0], "-shared") == 0) {
        if (tr[1] == NULL) {
            printf("Faltan argumentos\n");
            return;
        }
        int key = atoi(tr[1]);
        doDeallocateShared((key_t)key, MemoriaAlocada);
    }
    // Caso delkey
    else if (strcmp(tr[0], "-delkey") == 0) {
        if (tr[1] == NULL) {
            printf("Faltan argumentos\n");
            return;
        }
        char *fin;
        long key = strtol(tr[1], &fin, 10);

        if (fin == tr[1] || key == 0) {
            printf("delkey necesita clave_valida\n");
            return;
        }
        doDeallocateDelkey(key);
    }
    // Caso mmap
    else if (strcmp(tr[0], "-mmap") == 0) {
        if (tr[1] == NULL) {
            printf("Faltan argumentos\n");
            return;
        }
        doDeallocateMmap(tr[1], MemoriaAlocada);
    }
    // Caso Addr
    else {
        void *addr = (void *)(uintptr_t)strtoull(tr[0], NULL, 16);
        doDeallocateAddress(addr, MemoriaAlocada);
    }
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

int Cmd_memdump(char *tr[]) {

    if (tr[0] == NULL || tr[1] == NULL) {
        printf("usage: memdump <addr> <size>\n");
        return -1;
    }

    void *addr = (void *)(uintptr_t)strtoull(tr[0], NULL, 16);
    int siz = atoi(tr[1]);

    dumpMemory(addr, siz);

    return 0;
}
