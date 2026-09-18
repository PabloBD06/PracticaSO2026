//Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <ctype.h>
#define PATH_MAX 4096
#define Maxsize 128
#define Maxtokens 32
#define Maxhis 4096
#define MAX_OPEN_FILES 4096
#define TAMANO 1024
bool terminado = false;//finaliza el shell cuando pasa a true
// Variables para -vars
int ext_uninit1, ext_uninit2, ext_uninit3;
int ext_init1 = 1, ext_init2 = 2, ext_init3 = 3;
void f1(){};
void f2(){};
void f3(){};


typedef enum {MALLOC, MMAP, SHARED} block_type;

struct mem_block {
    void *addr;            // Dirección del bloque
    size_t size;           // Tamaño
    time_t timestamp;      // Momento de allocation
    block_type type;       // malloc, mmap, shared

    /* Campos extra */
    char file[256];        // para mmap
    int fd;                // file descriptor para mmap
    key_t key;             // para shared

    struct mem_block *next;
};

struct mem_block *memlist = NULL;

void add_block(struct mem_block *b) {
    b->next = memlist;
    memlist = b;
}

struct mem_block *find_block_by_addr(void *addr) {
    struct mem_block *p = memlist;
    while (p) {
        if (p->addr == addr) return p;
        p = p->next;
    }
    return NULL;
}

void remove_block(struct mem_block *b) {
    struct mem_block **pp = &memlist;
    while (*pp) {
        if (*pp == b) {
            *pp = b->next;
            free(b);
            return;
        }
        pp = &((*pp)->next);
    }
}

// ─── Gestión de ficheros abiertos ───
typedef struct { //Struct con la informacion relevante que tiene un archivo
    int fd;//file descriptor
    char *filename;//nombre del archivo
    int flags; //opciones con las que se abrio el archivo
} OpenFile;

OpenFile open_files[MAX_OPEN_FILES];//Lista de ficheros abiertos
int open_file_count = 0;//numero actual de archivos abiertos

// ─── Parámetros de 'dir' ───
typedef enum { SHORT_FORMAT, LONG_FORMAT } DirFormat;
typedef enum { NO_LINK, LINK } LinkFormat;
typedef enum { HIDDEN_NO, HIDDEN_YES } HiddenFormat;
typedef enum { NO_REC, RECURSE_AFTER, RECURSE_BEFORE } RecurseFormat;

DirFormat dir_format = SHORT_FORMAT;
LinkFormat dir_link_fmt = NO_LINK;     
HiddenFormat dir_hidden_fmt = HIDDEN_NO; 
RecurseFormat dir_recurse_fmt = NO_REC;  

//Funciones auxiliares del sistema de ficheros

//Borra el archivo abierto con file descriptor 'fd' de la lista y reorganiza la lista
void DeleteOpenFile(int fd){
    for (int i = 0; i < open_file_count; i++) {
        if (open_files[i].fd == fd) {
            free(open_files[i].filename);
            for (int j = i; j < open_file_count - 1; j++)
                open_files[j] = open_files[j + 1];
            open_file_count--;
            printf("Descriptor %d cerrado y eliminado de la lista.\n", fd);
            return;
        }
    }
    printf("El descriptor %d no estaba en la lista.\n", fd);
}

//Añade a la lista el archivo de filedescriptor 'fd', nombre 'filename' y de flags 'mode'
void AddOpenFile(int fd, const char *filename, int mode) {
    // Si la lista está llena, cerramos el primer descriptor y liberamos memoria
    if (open_file_count >= MAX_OPEN_FILES) {
        close(open_files[0].fd);
        free(open_files[0].filename);

        // Movemos los elementos hacia adelante para hacer hueco
        memmove(&open_files[0], &open_files[1], (MAX_OPEN_FILES - 1) * sizeof(OpenFile));
        open_file_count--;
    }

    // Añadimos el nuevo archivo al final de la lista
    open_files[open_file_count].fd = fd;
    open_files[open_file_count].filename = strdup(filename);
    open_files[open_file_count].flags = mode;
    open_file_count++;
}

//Busca en la lista el nombre del archivo de filedescriptor fd
char *GetFileNameFromFD(int fd) {
    for (int i = 0; i < open_file_count; i++) {
        if (open_files[i].fd == fd)
            return open_files[i].filename;
    }
    return NULL; // no encontrado
}

//Historial de comandos
char *historic[Maxhis];  // Lista de comandos usados
int historic_count = 0;  // cantidad actual de comandos

//Añade un comando a la lista de comandos
void AddHistoric(const char *cmd) {
    // Evitar almacenar "historic N" para prevenir bucles
    // N puede ser positivo o negativo, y también ignoramos -count y -clear
    if (strncmp(cmd, "historic ", 9) == 0) {
        const char *arg = cmd + 9;
        if (*arg == '-' || isdigit(*arg) || strncmp(arg, "count", 5) == 0 || strncmp(arg, "clear", 5) == 0) {
            return; // No añadimos comandos de tipo historic N, -N, -count o -clear
        }
    }

    char *copy = strdup(cmd);
    if (copy == NULL) { perror("strdup"); return; }

    if (historic_count < Maxhis) {
        historic[historic_count++] = copy;
    } else {
        free(historic[0]);
        for (int i = 1; i < Maxhis; i++)
            historic[i-1] = historic[i];
        historic[Maxhis-1] = copy;
    }
}



//Comandos del shell
void Cmd_authors(char *tokens[]){
    
    if (tokens[1] && !strcmp(tokens[1], "-l"))
        printf("pablo.bea.dopazo\neva.lopez.rodriguez1\n");
    else if (tokens[1] && !strcmp(tokens[1], "-n"))
        printf("Pablo Bea Dopazo\npersona2\n");
    else
        printf("Pablo Bea Dopazo(pablo.bea.dopazo)\npersona2\n");
}

void Cmd_getpid(char *tokens[]) {
     if (tokens[1] && !strcmp(tokens[1], "-p"))
        printf("%d\n", getppid());
    else
        printf("%d\n", getpid());
}

void Cmd_getcwd(){
     char path[1024];
    if (getcwd(path, sizeof(path)) != NULL)
            printf("Directorio actual: %s\n", path);
        else
            perror("getcwd");
}

void Cmd_chdir(char *tokens[]) {
    if (tokens[1] == NULL) {
        // sin argumentos → mostrar directorio actual
        Cmd_getcwd();
    } else {
        // con argumento → intentar cambiar
        if (chdir(tokens[1]) != 0)
            perror("chdir");
    }
}

void Cmd_hour() {
    time_t t = time(NULL);              
    struct tm *tm_info = localtime(&t);  

    char hora[9];                         // hh:mm:ss +'\0'
    strftime(hora, sizeof(hora), "%H:%M:%S", tm_info);

    printf("%s\n", hora);
}

void Cmd_date(char *tokens[]){
    if (tokens[1] && !strcmp(tokens[1], "-t")){
        Cmd_hour();
    }else{
        time_t t = time(NULL);              
        struct tm *tm_info = localtime(&t); 

        char fecha[11];  // DD/MM/YYYY + '\0'

        strftime(fecha, sizeof(fecha), "%d/%m/%Y", tm_info);
        if(tokens[1] && !strcmp(tokens[1], "-d")){
             printf("%s\n", fecha);
        }else{
             printf("%s\n", fecha);
             Cmd_hour();
        }
    }
}
//Las funciones TrocearCadena y procesarEntrada se definen más abajo, y para evitar warnings declaramos las cabeceras antes de usarlas
int TrocearCadena(char *cadena, char *tokens[]);
void procesarEntrada(char *tr, char *tokens[]);
void Cmd_historic(char *tokens[]) {
    if (tokens[1] == NULL) {
        // Sin parámetros: mostrar todos los comandos
        for (int i = 0; i < historic_count; i++)
            printf("%d %s\n", i + 1, historic[i]);
        return;
    }

    if (strcmp(tokens[1], "-count") == 0) {
        printf("%d current commands\n", historic_count);
        return;
    }

    if (strcmp(tokens[1], "-clear") == 0) {
        for (int i = 0; i < historic_count; i++)
            free(historic[i]);
        historic_count = 0;
        return;
    }

    int n = atoi(tokens[1]);

    if (n < 0) {
        // historic -N: mostrar últimos N comandos
        int start = (historic_count + n > 0) ? historic_count + n : 0;
        for (int i = start; i < historic_count; i++)
            printf("%d %s\n", i + 1, historic[i]);
        return;
    }

    if (n > 0 && n <= historic_count) {
        // historic N: repetir comando N
        char *cmd_to_repeat = strdup(historic[n - 1]);
        if (!cmd_to_repeat) { perror("strdup"); return; }

        char *tokens_repeat[Maxtokens];
        int numTokens = TrocearCadena(cmd_to_repeat, tokens_repeat);
        if (numTokens > 0) {
            procesarEntrada(cmd_to_repeat, tokens_repeat);
        }

        free(cmd_to_repeat);
        return;
    }

    fprintf(stderr, "historic: parámetro no válido\n");
}
void Cmd_listopen() {
    if (open_file_count == 0) {
        printf("No hay ficheros abiertos.\n");
        return;
    }
    printf("FD\tFLAGS\tFILENAME\n");
    for (int i = 0; i < open_file_count; i++)
        printf("%d\t%d\t%s\n", open_files[i].fd, open_files[i].flags, open_files[i].filename);
}

void Cmd_open(char *tokens[]) {
    int i, fd, mode = 0;

    if (tokens[1] == NULL) { // si no hay argumentos mostramos la lista
        Cmd_listopen();
        return;
    }

    // Mapeo de modos
    for (i = 2; tokens[i] != NULL; i++) {
        if (!strcmp(tokens[i], "cr")) mode |= O_CREAT;
        else if (!strcmp(tokens[i], "ex")) mode |= O_EXCL;
        else if (!strcmp(tokens[i], "ro")) mode |= O_RDONLY;
        else if (!strcmp(tokens[i], "wo")) mode |= O_WRONLY;
        else if (!strcmp(tokens[i], "rw")) mode |= O_RDWR;
        else if (!strcmp(tokens[i], "ap")) mode |= O_APPEND;
        else if (!strcmp(tokens[i], "tr")) mode |= O_TRUNC;
        else break;
    }
    //Modo por defecto si no se introduce modo
    if (mode == 0)
    mode = O_RDWR;

    // Abrir fichero
    if ((fd = open(tokens[1], mode, 0777)) == -1) {
        perror("Imposible abrir fichero");
        return;
    }

    // Guardar en la lista
    AddOpenFile(fd, tokens[1], mode);

    printf("Añadido: fd=%d, archivo=%s, modo=%d\n", fd, tokens[1], mode);
}



void Cmd_close(char *tr[]) {
    if (tr[1] == NULL) {
        printf("Argumentos insuficientes\n");
        return;
    }

    int fd = atoi(tr[1]);
    
    if (close(fd) == -1) {
        perror("Imposible cerrar descriptor");
        return;
    }

    // Eliminar del array open_files[]
    DeleteOpenFile(fd);
}

void Cmd_dup(char *tokens[]) {
    if (tokens[1] == NULL) {
        printf("Argumentos insuficientes\n");
        return;
    }

    int oldfd = atoi(tokens[1]);
    int newfd = dup(oldfd);
    if (newfd < 0) {
        perror("dup");
        return;
    }

    // Añadimos una entrada a la lista
    if (open_file_count < MAX_OPEN_FILES) {
        open_files[open_file_count].fd = newfd;
        open_files[open_file_count].filename = strdup("duplicated_fd");
        open_files[open_file_count].flags = 0;
        open_file_count++;
    }

    printf("Descriptor %d duplicado en %d\n", oldfd, newfd);
}


void Cmd_infosys() {
    struct utsname info;

    if (uname(&info) == -1) {
        perror("uname");
        return;
    }

    printf("Sistema operativo: %s\n", info.sysname);
    printf("Nombre de la máquina: %s\n", info.nodename);
    printf("Release/kernel: %s\n", info.release);
    printf("Versión del kernel: %s\n", info.version);
    printf("Arquitectura: %s\n", info.machine);
}

void Cmd_create(char *tokens[]) {
    if (tokens[1] == NULL) {
        printf("Argumentos insuficientes\n");
        return;
    }

    if (strcmp(tokens[1], "-f") == 0) {
        if (tokens[2] == NULL) {
            printf("Falta el nombre del fichero\n");
            return;
        }
        int fd = open(tokens[2], O_CREAT | O_EXCL | O_WRONLY, 0666);
        if (fd == -1) {
            perror("No se pudo crear el fichero");
        } else {
            printf("Fichero '%s' creado.\n", tokens[2]);
            close(fd);
        }
    } else {
        if (mkdir(tokens[1], 0777) == -1) {
            perror("No se pudo crear el directorio");
        } else {
            printf("Directorio '%s' creado.\n", tokens[1]);
        }
    }
}

void Cmd_setdirparams(char *tokens[]) {
    if (tokens[1] == NULL) {
        printf("Uso: setdirparams <long|short|link|nolink|hid|nohid|reca|recb|norec>\n");
        return;
    }

    if (strcmp(tokens[1], "long") == 0) dir_format = LONG_FORMAT;
    else if (strcmp(tokens[1], "short") == 0) dir_format = SHORT_FORMAT;
    else if (strcmp(tokens[1], "link") == 0) dir_link_fmt = LINK;
    else if (strcmp(tokens[1], "nolink") == 0) dir_link_fmt = NO_LINK;
    else if (strcmp(tokens[1], "hid") == 0) dir_hidden_fmt = HIDDEN_YES;
    else if (strcmp(tokens[1], "nohid") == 0) dir_hidden_fmt = HIDDEN_NO;
    else if (strcmp(tokens[1], "reca") == 0) dir_recurse_fmt = RECURSE_AFTER;
    else if (strcmp(tokens[1], "recb") == 0) dir_recurse_fmt = RECURSE_BEFORE;
    else if (strcmp(tokens[1], "norec") == 0) dir_recurse_fmt = NO_REC;
    else printf("Parámetro no reconocido: %s\n", tokens[1]);
}

void Cmd_getdirparams() {
    printf("Current dir parameters:\n");
    printf("Format: %s\n", dir_format == LONG_FORMAT ? "long" : "short");
    printf("Link: %s\n", dir_link_fmt == LINK ? "link" : "nolink");
    printf("Hidden: %s\n", dir_hidden_fmt == HIDDEN_YES ? "hid" : "nohid");
    printf("Recurse: %s\n",
           dir_recurse_fmt == RECURSE_AFTER ? "reca" :
           dir_recurse_fmt == RECURSE_BEFORE ? "recb" : "norec");
}



// ─── Funciones auxiliares ───
int EsDirectorio(char *dir) {
    struct stat s;
    if (lstat(dir, &s) == -1)
        return 0;
    return S_ISDIR(s.st_mode);
}
char LetraTF (mode_t m){
     switch (m&S_IFMT) {
        case S_IFSOCK: return 's';
        case S_IFLNK:  return 'l';
        case S_IFREG:  return '-';
        case S_IFBLK:  return 'b';
        case S_IFDIR:  return 'd';
        case S_IFCHR:  return 'c';
        case S_IFIFO:  return 'p';
        default:       return '?';
     }
}

char * ConvierteModo (mode_t m, char *permisos)
{
    strcpy (permisos,"---------- ");
    
    permisos[0]=LetraTF(m);
    if (m&S_IRUSR) permisos[1]='r';
    if (m&S_IWUSR) permisos[2]='w';
    if (m&S_IXUSR) permisos[3]='x';
    if (m&S_IRGRP) permisos[4]='r';
    if (m&S_IWGRP) permisos[5]='w';
    if (m&S_IXGRP) permisos[6]='x';
    if (m&S_IROTH) permisos[7]='r';
    if (m&S_IWOTH) permisos[8]='w';
    if (m&S_IXOTH) permisos[9]='x';
    if (m&S_ISUID) permisos[3]='s';
    if (m&S_ISGID) permisos[6]='s';
    if (m&S_ISVTX) permisos[9]='t';
    
    return permisos;
}

// ─── Función para imprimir la info de un fichero ───
// ─── Función para imprimir la info de un fichero ───
void PrintFileInfo(const char *path, const char *name) {
    struct stat st;
    char permisos[12];
    char link_target[PATH_MAX];

    bool show_link = (dir_link_fmt == LINK);

    if ((show_link ? lstat(path, &st) : stat(path, &st)) == -1) {
        perror(path);
        return;
    }

    bool long_format = (dir_format == LONG_FORMAT);

    if (!long_format) {
        printf("%-30s %ld bytes\n", name, (long)st.st_size);
    } else {
        ConvierteModo(st.st_mode, permisos);

        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        char timebuf[20];
        strftime(timebuf, sizeof(timebuf), "%d/%m/%Y-%H:%M", localtime(&st.st_mtime));

        printf("%s %3ld %-8s %-8s %8ld %s %s",
               permisos, (long)st.st_nlink,
               pw ? pw->pw_name : "?", gr ? gr->gr_name : "?",
               (long)st.st_size, timebuf, name);

        if (S_ISLNK(st.st_mode) && show_link) {
            ssize_t len = readlink(path, link_target, sizeof(link_target)-1);
            if (len != -1) {
                link_target[len] = '\0';
                printf(" -> %s", link_target);
            }
        }
        printf("\n");
    }
}

// ─── Función recursiva para listar directorios ───
void ListDirectory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) { perror(path); return; }

    struct dirent *entry;
    char fullpath[PATH_MAX];

    bool show_hidden = (dir_hidden_fmt == HIDDEN_YES);

    while ((entry = readdir(dir)) != NULL) {
        // Omitir ficheros ocultos si el flag no está activado
        if (!show_hidden && entry->d_name[0] == '.') continue;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) == -1) continue;

        // Recursividad antes si aplica
        if (S_ISDIR(st.st_mode) && dir_recurse_fmt == RECURSE_BEFORE) {
            ListDirectory(fullpath);
        }

        // Mostrar información del fichero
        PrintFileInfo(fullpath, entry->d_name);

        // Recursividad después si aplica
        if (S_ISDIR(st.st_mode) && dir_recurse_fmt == RECURSE_AFTER) {
            ListDirectory(fullpath);
        }
    }

    closedir(dir);
}

// ─── Función principal del comando dir ───
void Cmd_dir(char *tokens[]) {
    bool flag_d = false;
    int i = 1;

    // Leer solo la opción -d
    for (; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "-d") == 0) flag_d = true;
        else break;  // cualquier otro token se trata como fichero/directorio
    }

    // Si no hay más parámetros, listar el directorio actual según -d
    if (tokens[i] == NULL) {
        if (flag_d) ListDirectory(".");
        else PrintFileInfo(".", ".");
        return;
    }

    // Procesar ficheros o directorios indicados en los parámetros
    for (; tokens[i] != NULL; i++) {
        struct stat st;
        if (stat(tokens[i], &st) == -1) { perror(tokens[i]); continue; }

        if (S_ISDIR(st.st_mode)) {
            if (flag_d) {
                // Mostrar contenido del directorio según parámetros globales
                ListDirectory(tokens[i]);
            } else {
                // Mostrar solo info del directorio
                PrintFileInfo(tokens[i], tokens[i]);
            }
        } else {
            // Mostrar info de ficheros según parámetros globales
            PrintFileInfo(tokens[i], tokens[i]);
        }
    }
}

void borrarRecursivo(const char *path) {
    if (EsDirectorio((char *)path)) {
        DIR *dir = opendir(path);
        if (!dir) {
            perror(path);
            return;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char fullpath[PATH_MAX];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
            borrarRecursivo(fullpath);
        }

        closedir(dir);

        if (rmdir(path) == -1)
            perror(path);
        else
            printf("Directorio '%s' eliminado.\n", path);
    } else {
        if (unlink(path) == -1)
            perror(path);
        else
            printf("Archivo '%s' eliminado.\n", path);
    }
}

void Cmd_erase(char *tokens[]) {
    if (tokens[1] == NULL) {
        printf("Uso: erase n1 n2 ...\n");
        return;
    }

    for (int i = 1; tokens[i] != NULL; i++) {
        if (EsDirectorio(tokens[i])) {
            if (rmdir(tokens[i]) == -1) perror(tokens[i]);
            else printf("Directorio '%s' eliminado.\n", tokens[i]);
        } else {
            if (unlink(tokens[i]) == -1) perror(tokens[i]);
            else printf("Archivo '%s' eliminado.\n", tokens[i]);
        }
    }
}

void Cmd_delrec(char *tokens[]) {
    if (tokens[1] == NULL) {
        printf("Uso: delrec n1 n2 ...\n");
        return;
    }
    for (int i = 1; tokens[i] != NULL; i++)
        borrarRecursivo(tokens[i]);
}

void Cmd_lseek(char *tokens[]) {
    if (tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL) {
        printf("Uso: lseek df off ref\n");
        printf("ref puede ser SEEK_SET, SEEK_CUR o SEEK_END\n");
        return;
    }

    int fd = atoi(tokens[1]);
    off_t offset = atoll(tokens[2]);
    int whence;

    if (strcmp(tokens[3], "SEEK_SET") == 0) {
        whence = SEEK_SET;
    } else if (strcmp(tokens[3], "SEEK_CUR") == 0) {
        whence = SEEK_CUR;
    } else if (strcmp(tokens[3], "SEEK_END") == 0) {
        whence = SEEK_END;
    } else {
        printf("Referencia no válida: %s\n", tokens[3]);
        return;
    }

    off_t newpos = lseek(fd, offset, whence);
    if (newpos == (off_t)-1) {
        perror("lseek");
        return;
    }

    printf("Nueva posición del puntero: %lld bytes\n", (long long)newpos);
}

void Cmd_writestr(char *tokens[]) {
    if (tokens[1] == NULL || tokens[2] == NULL) {
        printf("Uso: writestr df str\n");
        return;
    }

    int fd = atoi(tokens[1]);
    char buffer[1024] = "";
    for (int i = 2; tokens[i] != NULL; i++) {
        strcat(buffer, tokens[i]);
        if (tokens[i + 1] != NULL) strcat(buffer, " ");
    }

    ssize_t written = write(fd, buffer, strlen(buffer));
    if (written == -1) {
        perror("write");
        return;
    }

    printf("%zd bytes escritos en el descriptor %d.\n", written, fd);
}
void print_malloc_blocks() {
    struct mem_block *p = memlist;
    while (p) {
        if (p->type == MALLOC)
            printf("malloc: addr=%p size=%zu time=%s",
                    p->addr, p->size, ctime(&p->timestamp));
        p = p->next;
    }
}
void Cmd_malloc(char *tokens[]) {

    /* LISTAR: malloc */
    if (!tokens[1]) {
        print_malloc_blocks();
        return;
    }

    /* LIBERAR: malloc -free n */
    if (strcmp(tokens[1], "-free") == 0) {

        if (!tokens[2]) {
            printf("Usage: malloc -free size\n");
            return;
        }

        size_t size = atol(tokens[2]);
        struct mem_block *p = memlist;

        while (p) {
            if (p->type == MALLOC && p->size == size) {

                free(p->addr);
                remove_block(p);
                printf("Freed malloc block of %zu bytes\n", size);
                return;
            }
            p = p->next;
        }

        printf("No malloc block of size %zu found\n", size);
        return;
    }

    /* ALLOCAR: malloc n */
    size_t size = atol(tokens[1]);

    if (size <= 0) {
        printf("malloc: size must be > 0\n");
        return;
    }

    void *addr = malloc(size);

    if (!addr) {
        perror("malloc");
        return;
    }

    struct mem_block *b = malloc(sizeof(struct mem_block));

    b->addr      = addr;
    b->size      = size;
    b->timestamp = time(NULL);
    b->type      = MALLOC;

    b->file[0] = '\0';
    b->fd      = -1;
    b->key     = -1;

    add_block(b);

    printf("Allocated %zu bytes at %p\n", size, addr);
}


void Cmd_mmap(char *tokens[]) {
    /* LISTAR */
    if (!tokens[1]) {
        struct mem_block *p = memlist;
        while (p) {
            if (p->type == MMAP)
                printf("mmap: %p (%zu bytes) file=%s %s",
                       p->addr, p->size, p->file, ctime(&p->timestamp));
            p = p->next;
        }
        return;
    }

    /* LIBERAR mmap -free file */
    if (strcmp(tokens[1], "-free") == 0) {
        if (!tokens[2]) {
            printf("Usage: mmap -free file\n");
            return;
        }

        struct mem_block *p = memlist;
        while (p) {
            if (p->type == MMAP && strcmp(p->file, tokens[2]) == 0) {
                munmap(p->addr, p->size);
                close(p->fd);
                remove_block(p);
                printf("Unmapped file %s\n", tokens[2]);
                return;
            }
            p = p->next;
        }

        printf("File %s not mapped\n", tokens[2]);
        return;
    }

    /* mmap file perm */
    if (!tokens[1] || !tokens[2]) {
        printf("Usage: mmap file perm\n");
        return;
    }

    char *file = tokens[1];
    char *perm = tokens[2];

    int prot = 0;
    if (perm[0] == 'r') prot |= PROT_READ;
    if (perm[1] == 'w') prot |= PROT_WRITE;
    if (perm[2] == 'x') prot |= PROT_EXEC;

    int fd = open(file, O_RDWR);
    if (fd < 0) {
        perror("open");
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return;
    }

    void *addr = mmap(NULL, st.st_size, prot, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return;
    }

    struct mem_block *b = malloc(sizeof(struct mem_block));
    b->addr = addr;
    b->size = st.st_size;
    b->timestamp = time(NULL);
    b->type = MMAP;
    strncpy(b->file, file, 255);
    b->file[255] = '\0';
    b->fd = fd;
    b->key = -1;
    add_block(b);

    printf("Mapped file %s at %p (%zu bytes)\n", file, addr, b->size);
}

void Cmd_shared(char *tokens[]) {
    /* LISTAR */
    if (!tokens[1]) {
        struct mem_block *p = memlist;
        while (p) {
            if (p->type == SHARED)
                printf("shared: %p (%zu bytes) key=%d %s",
                       p->addr, p->size, p->key, ctime(&p->timestamp));
            p = p->next;
        }
        return;
    }

    /* shared -create key size */
    if (strcmp(tokens[1], "-create") == 0) {
        if (!tokens[2] || !tokens[3]) {
            printf("Usage: shared -create key size\n");
            return;
        }

        key_t key = (key_t) atoi(tokens[2]);
        size_t size = atol(tokens[3]);

        int shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0666);
        if (shmid < 0) {
            perror("shmget");
            return;
        }

        void *addr = shmat(shmid, NULL, 0);
        if (addr == (void *) -1) {
            perror("shmat");
            return;
        }

        struct mem_block *b = malloc(sizeof(struct mem_block));
        b->addr = addr;
        b->size = size;
        b->timestamp = time(NULL);
        b->type = SHARED;
        b->key = key;
        b->fd = -1;
        b->file[0] = '\0';
        add_block(b);

        printf("Created and attached shared memory key=%d size=%zu at %p\n",
               key, size, addr);
        return;
    }

    /* shared -free key */
    if (strcmp(tokens[1], "-free") == 0) {
        if (!tokens[2]) {
            printf("Usage: shared -free key\n");
            return;
        }

        key_t key = atoi(tokens[2]);
        struct mem_block *p = memlist;

        while (p) {
            if (p->type == SHARED && p->key == key) {
                shmdt(p->addr);
                remove_block(p);
                printf("Detached shared memory key=%d\n", key);
                return;
            }
            p = p->next;
        }

        printf("Shared key %d not attached\n", key);
        return;
    }

    /* shared -delkey key */
    if (strcmp(tokens[1], "-delkey") == 0) {
        if (!tokens[2]) {
            printf("Usage: shared -delkey key\n");
            return;
        }

        key_t key = atoi(tokens[2]);
        int shmid = shmget(key, 0, 0);
        if (shmid < 0) {
            perror("shmget");
            return;
        }

        if (shmctl(shmid, IPC_RMID, NULL) < 0)
            perror("shmctl");
        else
            printf("Deleted key %d\n", key);

        return;
    }

    /* shared key → attach existing block */
    key_t key = atoi(tokens[1]);
    int shmid = shmget(key, 0, 0666);

    if (shmid < 0) {
        perror("shmget");
        return;
    }

    void *addr = shmat(shmid, NULL, 0);
    if (addr == (void *) -1) {
        perror("shmat");
        return;
    }

    struct shmid_ds ds;
    shmctl(shmid, IPC_STAT, &ds);

    struct mem_block *b = malloc(sizeof(struct mem_block));
    b->addr = addr;
    b->size = ds.shm_segsz;
    b->timestamp = time(NULL);
    b->type = SHARED;
    b->key = key;
    b->fd = -1;
    b->file[0] = '\0';

    add_block(b);

    printf("Attached shared key=%d at %p (%zu bytes)\n", key, addr, b->size);
}
void Cmd_free(char *tokens[]) {
    if (!tokens[1]) {
        printf("Usage: free addr\n");
        return;
    }

    void *addr = (void *) strtoull(tokens[1], NULL, 16);
    struct mem_block *b = find_block_by_addr(addr);

    if (!b) {
        printf("No allocated block at address %p\n", addr);
        return;
    }

    if (b->type == MALLOC) {
        free(b->addr);
    }
    else if (b->type == MMAP) {
        munmap(b->addr, b->size);
        close(b->fd);
    }
    else if (b->type == SHARED) {
        shmdt(b->addr);
    }

    remove_block(b);
    printf("Freed block at %p\n", addr);
}
void Cmd_memfill(char *tokens[]) {
    if (!tokens[1] || !tokens[2] || !tokens[3]) {
        printf("Usage: memfill addr cont char\n");
        return;
    }

    unsigned char *addr = (unsigned char *) strtoull(tokens[1], NULL, 16);
    long cont = atol(tokens[2]);

    if (cont <= 0) {
        printf("memfill: invalid count\n");
        return;
    }

    unsigned char c = tokens[3][0];

    for (long i = 0; i < cont; i++) {
        addr[i] = c;   // SEGFAULT permitido según el enunciado
    }

    printf("Filled %ld bytes at %p with '%c'\n", cont, addr, c);
}
void Cmd_memdump(char *tokens[]) {
    if (!tokens[1] || !tokens[2]) {
        printf("Usage: memdump addr cont\n");
        return;
    }

    unsigned char *addr = (unsigned char *) strtoull(tokens[1], NULL, 16);
    long cont = atol(tokens[2]);

    if (cont <= 0) {
        printf("memdump: invalid count\n");
        return;
    }

    for (long i = 0; i < cont; i++) {
        unsigned char c = addr[i];

        printf("%02x ", c);

        if (c == '\n') printf("'\\n'");
        else if (c == '\t') printf("'\\t'");
        else if (c == '\r') printf("'\\r'");
        else if (isprint(c)) printf("'%c'", c);
        else printf("   ");

        printf("\n");
    }
}
void mem_funcs() {
    printf("Program functions:\n");
    printf("  f1: %p\n", f1);
    printf("  f2: %p\n", f2);
    printf("  f3: %p\n", f3);

    printf("\nLibrary functions:\n");
    printf("  malloc: %p\n", malloc);
    printf("  printf: %p\n", printf);
    printf("  time:   %p\n", time);
}
void mem_vars() {
    static int stat_uninit1, stat_uninit2, stat_uninit3;
    static int stat_init1 = 10, stat_init2 = 20, stat_init3 = 30;

    int auto1 = 1, auto2 = 2, auto3 = 3;

    printf("External uninitialized: %p %p %p\n",
        &ext_uninit1, &ext_uninit2, &ext_uninit3);

    printf("External initialized:   %p %p %p\n",
        &ext_init1, &ext_init2, &ext_init3);

    printf("Static uninitialized:   %p %p %p\n",
        &stat_uninit1, &stat_uninit2, &stat_uninit3);

    printf("Static initialized:     %p %p %p\n",
        &stat_init1, &stat_init2, &stat_init3);

    printf("Automatic:              %p %p %p\n",
        &auto1, &auto2, &auto3);
}
void mem_blocks() {
    struct mem_block *p = memlist;

    while (p) {
        printf("Addr: %p | Size: %zu | Time: %ld | ", p->addr, p->size, p->timestamp);

        switch (p->type) {
        case MALLOC:
            printf("Type: MALLOC\n");
            break;

        case MMAP:
            printf("Type: MMAP | File: %s | FD: %d\n", p->file, p->fd);
            break;

        case SHARED:
            printf("Type: SHARED | Key: %d\n", p->key);
            break;
        }

        p = p->next;
    }
}
void mem_pmap() {
    char cmd[128];
    sprintf(cmd, "pmap %d", getpid());
    system(cmd);
}
void Cmd_mem(char *tokens[]) {
    if (!tokens[1] || strcmp(tokens[1], "-all") == 0) {
        mem_funcs();
        printf("\n");
        mem_vars();
        printf("\n");
        mem_blocks();
        return;
    }

    if (strcmp(tokens[1], "-funcs") == 0) {
        mem_funcs();
        return;
    }

    if (strcmp(tokens[1], "-vars") == 0) {
        mem_vars();
        return;
    }

    if (strcmp(tokens[1], "-blocks") == 0) {
        mem_blocks();
        return;
    }

    if (strcmp(tokens[1], "-pmap") == 0) {
        mem_pmap();
        return;
    }

    printf("mem: unknown option '%s'\n", tokens[1]);
}
void Cmd_readfile(char *tokens[]) {
    if (!tokens[1] || !tokens[2] || !tokens[3]) {
        fprintf(stderr, "Usage: readfile file addr cont\n");
        return;
    }

    char *filename = tokens[1];
    void *addr = (void *) strtoull(tokens[2], NULL, 0);
    size_t cont = strtoul(tokens[3], NULL, 10);

    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return;
    }

    size_t n = fread(addr, 1, cont, f);
    if (n < cont) {
        if (feof(f))
            fprintf(stderr, "Reached end of file after reading %zu bytes\n", n);
        else
            perror("fread");
    }

    fclose(f);
    printf("Read %zu bytes from %s into %p\n", n, filename, addr);
}
void Cmd_writefile(char *tokens[]) {
    if (!tokens[1] || !tokens[2] || !tokens[3]) {
        fprintf(stderr, "Usage: writefile file addr cont\n");
        return;
    }

    char *filename = tokens[1];
    void *addr = (void *) strtoull(tokens[2], NULL, 0);
    size_t cont = strtoul(tokens[3], NULL, 10);

    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        return;
    }

    size_t n = fwrite(addr, 1, cont, f);
    if (n < cont) {
        perror("fwrite");
    }

    fclose(f);
    printf("Wrote %zu bytes from %p to %s\n", n, addr, filename);
}
void Cmd_read(char *tokens[]) {
    if (!tokens[1] || !tokens[2] || !tokens[3]) {
        fprintf(stderr, "Usage: read fd addr cont\n");
        return;
    }

    int fd = strtoul(tokens[1], NULL, 0);
    void *addr = (void *) strtoull(tokens[2], NULL, 0);
    size_t cont = strtoul(tokens[3], NULL, 10);

    ssize_t n = read(fd, addr, cont);
    if (n == -1) {
        perror("read");
        return;
    }

    printf("Read %zd bytes from fd %d into %p\n", n, fd, addr);
}
void Cmd_write(char *tokens[]) {
    if (!tokens[1] || !tokens[2] || !tokens[3]) {
        fprintf(stderr, "Usage: write fd addr cont\n");
        return;
    }

    int fd = strtoul(tokens[1], NULL, 0);
    void *addr = (void *) strtoull(tokens[2], NULL, 0);
    size_t cont = strtoul(tokens[3], NULL, 10);

    ssize_t n = write(fd, addr, cont);
    if (n == -1) {
        perror("write");
        return;
    }

    printf("Wrote %zd bytes from %p to fd %d\n", n, addr, fd);
}
void recur(int k) {
        char automatico[TAMANO];
        static char estatico[TAMANO];

        printf("parametro: %3d (%p) array %p, arr estatico %p\n", 
               k, (void *)&k, (void *)automatico, (void *)estatico);

        if (k > 0)
        recur(k - 1);
 }
void Cmd_recurse(char *tokens[]) {

    if (!tokens[1]) {
        fprintf(stderr, "Usage: recurse n\n");
        return;
    }

    int n = strtol(tokens[1], NULL, 10);
    if (n < 0) {
        fprintf(stderr, "n must be >= 0\n");
        return;
    }

    recur(n);
}

void Cmd_help(char *tokens[]) {
    if (tokens[1] == NULL) {
        printf("Lista de comandos disponibles:\n");
        printf("  authors         getpid           chdir            getcwd\n");
        printf("  date            hour             historic         open\n");
        printf("  close           dup              listopen         infosys\n");
        printf("  help            exit             create           erase\n");
        printf("  delrec          dir              setdirparams     getdirparams\n");
        printf("  lseek           writestr         malloc           mmap\n");
        printf("  shared          free             memfill          memdump\n");
        printf("  mem             readfile         writefile        read\n");
        printf("  write           recurse\n");
        printf("\nUse 'help <comando>' para obtener información detallada.\n");
        return;
    }

    // Comandos clásicos P0 y P1 (ya existentes)
    if (strcmp(tokens[1], "authors") == 0) {
        printf("authors -> Muestra los nombres y logins de los autores del shell\n");
        printf("authors -n -> Muestra solo los nombres\n");
        printf("authors -l -> Muestra solo los logins\n");
    } 
    else if (strcmp(tokens[1], "getpid") == 0) {
        printf("getpid -> Muestra el PID de la shell\n");
        printf("getpid -p -> Muestra el PID del proceso padre de la shell\n");
    } 
    else if (strcmp(tokens[1], "chdir") == 0) {
        printf("chdir [dir] -> Cambia el directorio de trabajo actual de la shell a 'dir'\n");
        printf("Si se invoca sin argumentos, muestra el directorio actual\n");
    } 
    else if (strcmp(tokens[1], "getcwd") == 0) {
        printf("getcwd -> Muestra el directorio de trabajo actual de la shell\n");
    } 
    else if (strcmp(tokens[1], "date") == 0) {
        printf("date -> Muestra fecha y hora actuales\n");
        printf("date -d -> Muestra solo la fecha en formato DD/MM/YYYY\n");
        printf("date -t -> Muestra solo la hora en formato hh:mm:ss\n");
    } 
    else if (strcmp(tokens[1], "hour") == 0) {
        printf("hour -> Muestra la hora actual en formato hh:mm:ss\n");
    } 
    else if (strcmp(tokens[1], "historic") == 0) {
        printf("historic -> Muestra todos los comandos ejecutados con su número\n");
        printf("historic N -> Repite el comando número N\n");
        printf("historic -N -> Muestra solo los últimos N comandos\n");
        printf("historic -count -> Muestra el número total de comandos en el historial\n");
        printf("historic -clear -> Borra el historial completo\n");
    } 
    else if (strcmp(tokens[1], "open") == 0) {
        printf("open [fichero] [modo] -> Abre un fichero y lo añade a la lista de abiertos\n");
        printf("Modo: cr, ap, ex, ro, rw, wo, tr\n");
        printf("Si no se indican argumentos, muestra la lista de ficheros abiertos\n");
    } 
    else if (strcmp(tokens[1], "close") == 0) {
        printf("close [df] -> Cierra el descriptor de fichero indicado y lo elimina de la lista de abiertos\n");
    } 
    else if (strcmp(tokens[1], "dup") == 0) {
        printf("dup [df] -> Duplica el descriptor de fichero indicado\n");
    } 
    else if (strcmp(tokens[1], "listopen") == 0) {
        printf("listopen -> Muestra la lista de ficheros actualmente abiertos\n");
    } 
    else if (strcmp(tokens[1], "infosys") == 0) {
        printf("infosys -> Muestra información básica del sistema\n");
    } 
    else if (strcmp(tokens[1], "help") == 0) {
        printf("help [comando] -> Muestra la ayuda para el comando indicado\n");
    } 
    else if (strcmp(tokens[1], "exit") == 0 || strcmp(tokens[1], "quit") == 0 || strcmp(tokens[1], "bye") == 0) {
        printf("exit | quit | bye -> Termina la ejecución de la shell\n");
    } 
    else if (strcmp(tokens[1], "create") == 0) {
        printf("create -f <nombre> -> Crea un fichero\n");
        printf("create <nombre> -> Crea un directorio\n");
    } 
    else if (strcmp(tokens[1], "erase") == 0) {
        printf("erase n1 n2 ... -> Elimina ficheros o directorios vacíos\n");
    } 
    else if (strcmp(tokens[1], "delrec") == 0) {
        printf("delrec n1 n2 ... -> Elimina ficheros o directorios no vacíos recursivamente\n");
    } 
    else if (strcmp(tokens[1], "lseek") == 0) {
        printf("lseek <df> <off> <ref> -> Mueve el puntero de lectura/escritura\n");
        printf("ref: SEEK_SET, SEEK_CUR, SEEK_END\n");
    } 
    else if (strcmp(tokens[1], "writestr") == 0) {
        printf("writestr <df> <cadena> -> Escribe la cadena en el fichero abierto\n");
    } 
    else if (strcmp(tokens[1], "setdirparams") == 0) {
        printf("setdirparams long|short, link|nolink, hid|nohid, reca|recb|norec\n");
    } 
    else if (strcmp(tokens[1], "getdirparams") == 0) {
        printf("getdirparams -> Muestra los parámetros actuales para 'dir'\n");
    } 
    else if (strcmp(tokens[1], "dir") == 0) {
        printf("dir n1 n2 ... -> Muestra información de ficheros/directorios\n");
    } 

    // ─── NUEVOS COMANDOS DE MEMORIA Y ARCHIVOS ───
    else if (strcmp(tokens[1], "malloc") == 0) {
        printf("malloc n -> Asigna un bloque de memoria de tamaño n\n");
        printf("malloc -free n -> Libera un bloque malloc de tamaño n\n");
        printf("malloc -> Lista los bloques malloc asignados\n");
    } 
    else if (strcmp(tokens[1], "mmap") == 0) {
        printf("mmap fich perm -> Mapea el archivo 'fich' con permisos 'perm' (rwx)\n");
        printf("mmap -free fich -> Desmapea el archivo 'fich'\n");
        printf("mmap -> Lista los bloques mmap asignados\n");
    } 
    else if (strcmp(tokens[1], "shared") == 0) {
        printf("shared -create cl n -> Crea memoria compartida con key cl y tamaño n\n");
        printf("shared cl -> Adjunta memoria compartida con key cl\n");
        printf("shared -free cl -> Desadjunta memoria compartida con key cl\n");
        printf("shared -delkey cl -> Elimina la memoria compartida del sistema\n");
        printf("shared -> Lista los bloques de memoria compartida\n");
    } 
    else if (strcmp(tokens[1], "free") == 0) {
        printf("free addr -> Libera el bloque en la dirección addr (malloc, shared, mmap)\n");
    } 
    else if (strcmp(tokens[1], "memfill") == 0) {
        printf("memfill addr cont ch -> Llena cont bytes en addr con el carácter ch\n");
    } 
    else if (strcmp(tokens[1], "memdump") == 0) {
        printf("memdump addr cont -> Muestra cont bytes de memoria desde addr en hexadecimal y ASCII\n");
    } 
    else if (strcmp(tokens[1], "mem") == 0) {
        printf("mem -funcs -> Direcciones de funciones\n");
        printf("mem -vars -> Direcciones de variables\n");
        printf("mem -blocks -> Lista de bloques asignados\n");
        printf("mem -all -> Todo lo anterior\n");
        printf("mem -pmap -> Muestra pmap del proceso\n");
    } 
    else if (strcmp(tokens[1], "readfile") == 0) {
        printf("readfile file addr cont -> Lee cont bytes del archivo file a addr\n");
    } 
    else if (strcmp(tokens[1], "writefile") == 0) {
        printf("writefile file addr cont -> Escribe cont bytes de addr al archivo file\n");
    } 
    else if (strcmp(tokens[1], "read") == 0) {
        printf("read df addr cont -> Igual que readfile pero usando descriptor df abierto\n");
    } 
    else if (strcmp(tokens[1], "write") == 0) {
        printf("write df addr cont -> Igual que writefile pero usando descriptor df abierto\n");
    } 
    else if (strcmp(tokens[1], "recurse") == 0) {
        printf("recurse n -> Ejecuta función recursiva n veces mostrando direcciones de parámetros y arrays\n");
    } 
    else {
        printf("Comando no reconocido: %s\n", tokens[1]);
    }
}

//Funciones del bucle del shell
//Cierra los ficheros abiertos y libera el historial de comandos al salir del shell
void Cleanup() {
    for (int i = 0; i < historic_count; i++)
        free(historic[i]);
    for (int i = 0; i < open_file_count; i++) {
        close(open_files[i].fd);
        free(open_files[i].filename);
    }
}

void imprimirPrompt(){
    printf("Shell>");
     fflush(stdout);
}
void leerEntrada(char *tr, int maxsize) {
    if (fgets(tr, maxsize, stdin) == NULL) {
        printf("\nSaliendo...\n");
        exit(0);
    }
}
int TrocearCadena(char * cadena, char * tokens[]){ int i=1;
    if ((tokens[0]=strtok(cadena," \n\t"))==NULL)
        return 0;
    while ((tokens[i]=strtok(NULL," \n\t"))!=NULL)
        i++;
    return i;
}

void procesarEntrada(char *tr, char *tokens[]) {
    while (*tr == ' ' || *tr == '\t' || *tr == '\n')
        tr++;
    if (*tr == '\0') return;

    AddHistoric(tr);

    int numTokens = TrocearCadena(tr, tokens);
    if (numTokens == 0) return;

    // ─── Comandos internos ───
    if (strcmp(tokens[0], "exit") == 0 ||
        strcmp(tokens[0], "quit") == 0 ||
        strcmp(tokens[0], "bye") == 0) {

        terminado = true;

    } else if (strcmp(tokens[0], "authors") == 0) {
        Cmd_authors(tokens);

    } else if (strcmp(tokens[0], "getpid") == 0) {
        Cmd_getpid(tokens);

    } else if (strcmp(tokens[0], "chdir") == 0) {
        Cmd_chdir(tokens);

    } else if (strcmp(tokens[0], "getcwd") == 0) {
        Cmd_getcwd();

    } else if (strcmp(tokens[0], "date") == 0) {
        Cmd_date(tokens);

    } else if (strcmp(tokens[0], "hour") == 0) {
        Cmd_hour();

    } else if (strcmp(tokens[0], "historic") == 0) {
        Cmd_historic(tokens);

    } else if (strcmp(tokens[0], "open") == 0) {
        Cmd_open(tokens);

    } else if (strcmp(tokens[0], "close") == 0) {
        Cmd_close(tokens);

    } else if (strcmp(tokens[0], "dup") == 0) {
        Cmd_dup(tokens);

    } else if (strcmp(tokens[0], "listopen") == 0) {
        Cmd_listopen();

    } else if (strcmp(tokens[0], "infosys") == 0) {
        Cmd_infosys();

    } else if (strcmp(tokens[0], "help") == 0) {
        Cmd_help(tokens);

    // ─── Comandos P1 ───
    } else if (strcmp(tokens[0], "create") == 0) {
        Cmd_create(tokens);

    } else if (strcmp(tokens[0], "erase") == 0) {
        Cmd_erase(tokens);

    } else if (strcmp(tokens[0], "delrec") == 0) {
        Cmd_delrec(tokens);

    } else if (strcmp(tokens[0], "lseek") == 0) {
        Cmd_lseek(tokens);

    } else if (strcmp(tokens[0], "writestr") == 0) {
        Cmd_writestr(tokens);

    } else if (strcmp(tokens[0], "dir") == 0) {
        Cmd_dir(tokens);

    } else if (strcmp(tokens[0], "setdirparams") == 0) {
        Cmd_setdirparams(tokens);

    } else if (strcmp(tokens[0], "getdirparams") == 0) {
        Cmd_getdirparams();

    // ─── Memoria (P2) ───
    } else if (strcmp(tokens[0], "malloc") == 0) {
        Cmd_malloc(tokens);

    } else if (strcmp(tokens[0], "mmap") == 0) {
        Cmd_mmap(tokens);

    } else if (strcmp(tokens[0], "shared") == 0) {
        Cmd_shared(tokens);

    } else if (strcmp(tokens[0], "free") == 0) {
        Cmd_free(tokens);

    } else if (strcmp(tokens[0], "memfill") == 0) {
        Cmd_memfill(tokens);

    } else if (strcmp(tokens[0], "memdump") == 0) {
        Cmd_memdump(tokens);

    } else if (strcmp(tokens[0], "mem") == 0) {
        Cmd_mem(tokens);

    // ─── Lectura/escritura P2 ───
    } else if (strcmp(tokens[0], "readfile") == 0) {
        Cmd_readfile(tokens);

    } else if (strcmp(tokens[0], "writefile") == 0) {
        Cmd_writefile(tokens);

    } else if (strcmp(tokens[0], "read") == 0) {
        Cmd_read(tokens);

    } else if (strcmp(tokens[0], "write") == 0) {
        Cmd_write(tokens);

    // ─── Recursividad ───
    } else if (strcmp(tokens[0], "recurse") == 0) {
        Cmd_recurse(tokens);

    } else {
        printf("Comando no reconocido: %s\n", tokens[0]);
    }
}




//Main
int main(){
    char *tokens[Maxtokens];
    char cadena[Maxsize];
    while (!terminado){
    imprimirPrompt();
    leerEntrada(cadena, Maxsize);
    procesarEntrada(cadena,tokens);
    }
    Cleanup();
    return 0;
}