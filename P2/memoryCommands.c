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
#include "shellCommands.h"

#define BYTES_PER_LINE 20

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

int g_i1 = 1, g_i2 = 2, g_i3 = 3;
int g_n1, g_n2, g_n3;

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

void ListarMemoria(tList *MemoriaAlocada, int tipo) {

    if (tipo == -1) {
        printf("******Lista de bloques asignados para el proceso %d\n", getpid());
    }
    else if (tipo == MALLOC) {
        printf("******Lista de bloques asignados malloc para el proceso %d\n", getpid());
    }
    else if (tipo == SHARED) {
        printf("******Lista de bloques asignados shared para el proceso %d\n", getpid());
    }
    else if (tipo == MAPPED) {
        printf("******Lista de bloques asignados mmap para el proceso %d\n", getpid());
    }

    for (tPosL pos = first(*MemoriaAlocada); pos != LNULL; pos = next(pos, *MemoriaAlocada)) {

        memory *m = (memory *) getItem(pos, *MemoriaAlocada);

        // Si se ha solicitado un tipo concreto, ignorar los demás.
        if (tipo != -1 && (int)m->type != tipo) {
            continue;
        }

        char date[7];
        char hour[6];

        struct tm *tm_info = localtime(&m->allocationTime);

        if (tm_info == NULL) {
            continue;
        }

        strftime(date, sizeof(date), "%b %d", tm_info);
        strftime(hour, sizeof(hour), "%H:%M", tm_info);

        printf("     %p %15zu %s %s ",
               m->memoryAddress, m->size, date, hour);

        if (m->type == MALLOC) {
            printf("malloc\n");
        }
        else if (m->type == SHARED) {
            printf("shared (key %d)\n",
                   (int) m->u.shared.key);
        }
        else if (m->type == MAPPED) {
            printf("%s (descriptor %d)\n",
                   m->u.mapped.fileName,
                   m->u.mapped.fd);
        }
    }
}

/* ───────────────────── FUNCIONES AUXILIARES ───────────────────── */

int parseHexChar(const char *str, char *out) {
    // must start with "0x" or "0X"
    if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X'))
        return -1;

    const char *digits = str + 2;

    // need at least two valid hex digits (checking [0] first
    // also guarantees we never read past the string's '\0')
    if (!isxdigit((unsigned char)digits[0]) ||
        !isxdigit((unsigned char)digits[1]))
        return -1;

    char buf[3] = {digits[0], digits[1], '\0'};
    *out = (char)strtoul(buf, NULL, 16);
    return 0;
}

void *mallocLocal(size_t siz, tList *list) {
    if (siz == 0) {
        return NULL;
    }

    void *addr = malloc(siz);
    memory *item = malloc(sizeof(struct memory));

    if (item == NULL || addr == NULL) {
        free(addr);
        free(item);
        return NULL;
    }

    item->memoryAddress = addr;
    item->allocationTime = time(NULL);
    item->size = siz;
    item->type = MALLOC;

    if (!insertItem(item, LNULL, list)) {
        free(addr);
        LiberarMemoria(item);
        return NULL;
    }

    return addr;
}

void *createSharedLocal(key_t cl, size_t siz, tList *list) {
    if (siz == 0) {
        return NULL;
    }

    int shmid = shmget(cl, siz, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        return NULL;
    }
    void *addr = shmat(shmid, NULL, 0);
    if (addr == (void *)-1) {
        shmctl(shmid, IPC_RMID, NULL);
        return NULL;
    }

    memory *item = malloc(sizeof(struct memory));
    if (item == NULL) {
        shmdt(addr);
        shmctl(shmid, IPC_RMID, NULL);
        return NULL;
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
        return NULL;
    }

    return addr;
}

void *sharedLocal(key_t cl, tList *list) {
    int shmid = shmget(cl, 0, 0666);
    if (shmid == -1) {
        return NULL;
    }

    struct shmid_ds buf;
    if (shmctl(shmid, IPC_STAT, &buf) == -1) {
        return NULL;
    }

    void *addr = shmat(shmid, NULL, 0);
    if (addr == (void *)-1) {
        return NULL;
    }

    memory *item = malloc(sizeof(struct memory));
    if (item == NULL) {
        shmdt(addr);
        return NULL;
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
        return NULL;
    }

    return addr;
}

void *mmapLocal(char *fil, char *perm, tList *list, tList *FicherosAbiertos) {
    int prot = 0;

    if (strchr(perm, 'r')) prot |= PROT_READ;
    if (strchr(perm, 'w')) prot |= PROT_WRITE;
    if (strchr(perm, 'x')) prot |= PROT_EXEC;

    int oflags = strchr(perm, 'w') ? O_RDWR : O_RDONLY;

    int fd = open(fil, oflags);
    if (fd == -1)
        return NULL;

    struct stat st;
    if (fstat(fd, &st) == -1 || st.st_size <= 0) {
        close(fd);
        return NULL;
    }

    void *addr = mmap(NULL, st.st_size, prot, MAP_SHARED, fd, 0);

    if (addr == MAP_FAILED) {
        close(fd);
        return NULL;
    }

    memory *item = malloc(sizeof(struct memory));
    char *name = strdup(fil);

    if (item == NULL || name == NULL) {
        free(item);
        free(name);
        munmap(addr, st.st_size);
        close(fd);
        return NULL;
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
        return NULL;
    }

    // Añadimos el descriptor a la lista de ficheros abiertos
    if (!AñadirAFicherosAbiertos(fd, oflags, fil, FicherosAbiertos, true)) {

        // Eliminamos el último elemento (el que acabamos de insertar)
        deleteAtPosition(last(*list), list);

        LiberarMemoria(item);
        munmap(addr, st.st_size);
        close(fd);

        return NULL;
    }

    return addr;
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

void doDeallocateMmap(char *filename, tList *MemoriaAlocada, tList *FicherosAbiertos) {
    for (tPosL pos = first(*MemoriaAlocada);
         pos != LNULL;
         pos = next(pos, *MemoriaAlocada)) {

        tItemL item = getItem(pos, *MemoriaAlocada);
        memory *m = (memory *)item;

        if (m->type == MAPPED &&
            strcmp(m->u.mapped.fileName, filename) == 0) {

            // 1. Desmapear la región de memoria
            if (munmap(m->memoryAddress, m->size) == -1) {
                perror("munmap");
                return;
            }

            // 2. Buscar el descriptor en la lista de ficheros abiertos
            int fd = m->u.mapped.fd;
            tPosL posFile = findItemFunc(&fd, *FicherosAbiertos, matchByFd);

            if (posFile != LNULL) {
                OpenFile of = getItem(posFile, *FicherosAbiertos);

                // 3. Cerrar el descriptor
                if (close(fd) == -1) {
                    perror("close");
                    // El mapeo ya no existe, pero conservamos la entrada
                    // del descriptor porque no hemos confirmado su cierre.
                    of->IsMapped = false;
                } else {
                    // 4. Eliminar la entrada de la lista de ficheros
                    LiberaOpenFile(of);
                    deleteAtPosition(posFile, FicherosAbiertos);
                }
            } else {
                // El descriptor no estaba registrado en nuestra lista
                close(fd);
            }

            // 5. Eliminar el bloque de la lista de memoria
            LiberarMemoria(m);
            deleteAtPosition(pos, MemoriaAlocada);

            return;
        }
    }

    printf("No hay bloque de esa clave mapeado para el proceso\n");
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
    printf("No ejecutado: No such file or directory\n");
}

static void printAddress(void *addr) {
    printf("%p-> ", addr);
}

void fillMemory(void *addr, size_t siz, char character) {
    char *p = (char *)addr;
    size_t i = 0;

    while (i < siz) {
        p[i] = character;
        i++;
    }
}

void dumpMemory(void *addr, int siz) {
    unsigned char *p = addr;

    for (int offset = 0; offset < siz; offset += BYTES_PER_LINE) {

        int n = siz - offset;
        if (n > BYTES_PER_LINE) n = BYTES_PER_LINE;

        printAddress(p + offset);
        for (int i = 0; i < n; i++) {
            if (isprint(p[offset + i]))
                printf(" %c ", p[offset + i]);
            else
                printf("  ");
        }
        printf("\n");

        printAddress(p + offset);
        for (int i = 0; i < n; i++) {
            printf("%02x ", p[offset + i]);
        }
        printf("\n");
    }
}

void recurse(int n) {
    int array[2048];
    static int staticArray[2048];

    printf("parametro: %i (%p) array %p, array estatico %p\n", n, &n, array, staticArray);
    if (n > 0)
        recurse(n - 1);
}

static void PrintBlocks(tList *list) {
    char date[64];

    printf("******Lista de bloques asignados para el proceso %d\n", getpid());
    for (tPosL pos = first(*list); pos != LNULL; pos = next(pos, *list)) {
        struct memory *item = (struct memory *)getItem(pos, *list);
        struct tm *t = localtime(&item->allocationTime);
        strftime(date, sizeof(date), "%b %e %H:%M", t);

        printf("%20p %16zu %s ", item->memoryAddress, item->size, date);
        switch (item->type) {
        case 0:
            printf("malloc\n");
            break;
        case 1:
            printf("shared (key %d)\n", item->u.shared.key);
            break;
        case 2:
            printf("%s  (descriptor %d)\n", item->u.mapped.fileName, item->u.mapped.fd);
            break;
        default:
            printf("unknown\n");
        }
    }
}

static void PrintVars() {
    int l1 = 1, l2 = 2, l3 = 3;              
    static int s_i1 = 1, s_i2 = 2, s_i3 = 3; 
    static int s_n1, s_n2, s_n3;             

    printf("Variables locales       %p, %p, %p\n",
           (void *)&l1, (void *)&l2, (void *)&l3);
    printf("Variables globales      %p, %p, %p\n",
           (void *)&g_i1, (void *)&g_i2, (void *)&g_i3);
    printf("Var (N.I.)globales      %p, %p, %p\n",
           (void *)&g_n1, (void *)&g_n2, (void *)&g_n3);
    printf("Variables staticas      %p, %p, %p\n",
           (void *)&s_i1, (void *)&s_i2, (void *)&s_i3);
    printf("Var (N.I.)staticas      %p, %p, %p\n",
           (void *)&s_n1, (void *)&s_n2, (void *)&s_n3);
}

static void PrintFuncs() {
    printf("Funciones programa      %p, %16p, %16p\n",
           (void *)mmapLocal, (void *)PrintBlocks, (void *)PrintVars);
    
    printf("Funciones libreria      %p, %16p, %16p\n",
           (void *)printf, (void *)malloc, (void *)strcmp);
}

static void PrintPmap() {
    char command[50];
    sprintf(command, "pmap %d", getpid());
    system(command);
}

/* ───────────────────── COMANDOS DE MEMORIA ───────────────────── */

int Cmd_allocate(char *tr[], tList *list, tList *FicherosAbiertos) {
    if (tr[0] == NULL) {
        ListarMemoria(list, -1);
        return 0;
    }

    void *pointer;

    if (!strcmp(tr[0], "-malloc")) {
        size_t siz;

        if (tr[1] == NULL) {
            ListarMemoria(list, MALLOC);
            return 0;
        }

        siz = strtoull(tr[1], NULL, 10);

        if (siz <= 0) {
            fprintf(stderr, "allocate: -malloc needs a positive size\n");
            return -1;
        }

        pointer = mallocLocal(siz, list);

        if (pointer == NULL) {
            fprintf(stderr, "allocate: failed to reserve memory\n");
            return -1;
        }

        printf("Asignados %li bytes en %p\n", siz, pointer);

        return 0;

    } else if (!strcmp(tr[0], "-createshared")) {
        key_t cl;
        size_t siz;
        if(tr[1] == NULL){
            ListarMemoria(list, SHARED);
            return 0;
        }
        if (tr[2] == NULL) {
            fprintf(stderr, "allocate: -createshared needs <key> <size>\n");
            return -1;
        }

        cl = atoi(tr[1]);
        siz = strtoull(tr[2], NULL, 10);

        if (siz <= 0) {
            fprintf(stderr, "allocate: size needs to be greater than 0\n");
            return -1;
        }

        pointer = createSharedLocal(cl, siz, list);

        if (pointer == NULL) {
            fprintf(stderr, "allocate: failed to reserve memory\n");
            return -1;
        }

        printf("Asignados %li bytes en %p\n", siz, pointer);

        return 0;

    } else if (!strcmp(tr[0], "-shared")) {
        key_t cl;

        if (tr[1] == NULL) {
            ListarMemoria(list, SHARED);
            return 0;
        }

        cl = strtol(tr[1], NULL, 10);

        pointer = sharedLocal(cl, list);

        if (pointer == NULL) {
            fprintf(stderr, "allocate: failed to reserve memory\n");
            return -1;
        }

        printf("Memoria compartida de clave %i en %p\n", cl, pointer);

        return 0;

    } else if (!strcmp(tr[0], "-mmap")) {
        if (tr[1] == NULL) {
            ListarMemoria(list, MAPPED);
            return 0;
        }
        if (tr[2] == NULL) {
            fprintf(stderr, "allocate: -mmap needs <file> <perm>\n");
            return -1;
        }

        pointer = mmapLocal(tr[1], tr[2], list, FicherosAbiertos);

        if (pointer == NULL) {
            fprintf(stderr, "allocate: failed to reserve memory\n");
            return -1;
        }

        printf("fichero %s mapeado en %p\n", tr[1], pointer);

        return 0;

    } else {
        fprintf(stderr, "allocate: unknown option %s\n", tr[0]);
        return -1;
    }
}

void Cmd_deallocate(char *tr[], tList *MemoriaAlocada, tList *FicherosAbiertos) {
    // Sin argumentos-> Listar memoria
    if (tr[0] == NULL) {
        ListarMemoria(MemoriaAlocada, -1);
        return;
    }
    // Caso malloc
    if (strcmp(tr[0], "-malloc") == 0) {
        if (tr[1] == NULL) {
            ListarMemoria(MemoriaAlocada, MALLOC);
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
            ListarMemoria(MemoriaAlocada, SHARED);
            return;
        }
        int key = atoi(tr[1]);
        doDeallocateShared((key_t)key, MemoriaAlocada);
    }
    // Caso delkey
    else if (strcmp(tr[0], "-delkey") == 0) {
        if (tr[1] == NULL) {
            printf("delkey necesita clave_valida\n");
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
            ListarMemoria(MemoriaAlocada, MAPPED);
            return;
        }
        doDeallocateMmap(tr[1], MemoriaAlocada, FicherosAbiertos);
    }
    // Caso Addr
    else {
        void *addr = (void *)(uintptr_t)strtoull(tr[0], NULL, 16);
        doDeallocateAddress(addr, MemoriaAlocada);
    }
}

void Cmd_io(char *tr[]) {
    if (tr[0] == NULL) {
        printf("uso: i-o [read|write] ......");
        return;
    }
    // Caso read
    if (!strcmp(tr[0], "read")) {
        if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
            printf("Faltan parametros");
            return;
        }
        int fd = open(tr[1], O_RDONLY);
        if (fd == -1) {
            perror("open");
            return;
        }
        void *addr = (void *)(uintptr_t)strtoull(tr[2], NULL, 16);
        size_t cont = atoi(tr[3]);
        ssize_t bytesleidos = read(fd, addr, cont);
        if (bytesleidos == -1) {
            perror("read");
            close(fd);
            return;
        }
        if (close(fd) == -1) {
            perror("close");
            return;
        }
        printf("leidos %zd bytes de %s en %p\n", bytesleidos, tr[1], addr);
        return;
    }
    // Caso write
    if (!strcmp(tr[0], "write")) {
        if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
            printf("Faltan parametros");
            return;
        }
        if (!strcmp(tr[1], "-o")) {
            if (tr[4] == NULL) {
                printf("Faltan parametros");
                return;
            }
            int fd = open(tr[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd == -1) {
                perror("open");
                return;
            }
            void *addr = (void *)(uintptr_t)strtoull(tr[3], NULL, 16);
            int cont = atoi(tr[4]);
            ssize_t bytesescritos = write(fd, addr, cont);
            if (bytesescritos == -1) {
                perror("write");
                close(fd);
                return;
            }
            if (close(fd) == -1) {
                perror("close");
                return;
            }
            printf("escritos %zd bytes en %s desde %p\n", bytesescritos, tr[2], addr);
            return;
        } else {
            int fd = open(tr[1], O_WRONLY | O_CREAT | O_EXCL, 0666);
            if (fd == -1) {
                perror("open");
                return;
            }
            void *addr = (void *)(uintptr_t)strtoull(tr[2], NULL, 16);
            int cont = atoi(tr[3]);
            ssize_t bytesescritos = write(fd, addr, cont);
            if (bytesescritos == -1) {
                perror("write");
                close(fd);
                return;
            }
            if (close(fd) == -1) {
                perror("close");
                return;
            }
            printf("escritos %zd bytes en %s desde %p\n", bytesescritos, tr[1], addr);
            return;
        }
    }
    // si no ponen ni read ni write
    printf("uso: i-o [read|write] ......");
    return;
}

int Cmd_memory(char *tr[], tList *list) {
    if (tr == NULL || tr[0] == NULL) {
        PrintVars();
        PrintFuncs();
        PrintBlocks(list);
        return 0;
    }

    if (strcmp(tr[0], "-blocks") == 0) {
        PrintBlocks(list);
    } else if (strcmp(tr[0], "-funcs") == 0) {
        PrintFuncs();
    } else if (strcmp(tr[0], "-vars") == 0) {
        PrintVars();
    } else if (strcmp(tr[0], "-all") == 0) {
        PrintVars();
        PrintFuncs();
        PrintBlocks(list);
    } else if (strcmp(tr[0], "-pmap") == 0) {
        PrintPmap();
    } else {
        printf("Uso: memory [-blocks|-funcs|-vars|-all|-pmap]\n");
        return -1;
    }
    return 0;
}

void Cmd_read(char *tr[]) {
    if (tr[0] == NULL || tr[1] == NULL || tr[2] == NULL) {
        printf("Parametros incorrectos");
        return;
    }
    int fd = atoi(tr[0]);
    void *addr = (void *)(uintptr_t)strtoull(tr[1], NULL, 16);
    size_t cont = atoi(tr[2]);
    ssize_t bytesleidos = read(fd, addr, cont);
    if (bytesleidos == -1) {
        perror("read");
        return;
    }
    printf("escritos %zd bytes del descriptor %d en la direccion %p\n", bytesleidos, fd, addr);
    return;
}

void Cmd_write(char *tr[]) {
    if (tr[0] == NULL || tr[1] == NULL || tr[2] == NULL) {
        printf("Parametros incorrectos");
        return;
    }
    int fd = atoi(tr[0]);
    void *addr = (void *)(uintptr_t)strtoull(tr[1], NULL, 16);
    size_t cont = atoi(tr[2]);
    ssize_t bytesescritos = write(fd, addr, cont);
    if (bytesescritos == -1) {
        perror("write");
        return;
    }
    printf("escritos %zd bytes del descriptor %d en la direccion %p\n", bytesescritos, fd, addr);
    return;
}

int Cmd_recurse(char *tr[]) {
    if (tr[0] == NULL) {
        printf("usage: <n>\n");
        return -1;
    }

    int n = atoi(tr[0]);

    recurse(n);
    return 0;
}

int Cmd_memfill(char *tr[]) {
    if (tr[0] == NULL || tr[1] == NULL || tr[2] == NULL) {
        printf("usage: memfill <addr> <size> <hcode>\n");
        return -1;
    }

    void *addr = (void *)(uintptr_t)strtoull(tr[0], NULL, 16);
    size_t siz = strtoul(tr[1], NULL, 10);
    char c;
    parseHexChar(tr[2], &c);

    fillMemory(addr, siz, c);

    return 0;
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