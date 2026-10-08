// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

#include "comandosp1.h"
#include "dynamicList.h"

/* ───────────────────── GESTIÓN DE FICHEROS ABIERTOS ───────────────────── */

static void LiberaOpenFile(void *ptr) {
    OpenFile of = (OpenFile)ptr;

    if (of != NULL) {
        if (of->filename != NULL) {
            free(of->filename);
        }
        free(of);
    }
}

static bool AñadirAFicherosAbiertos(int fd, int mode, const char *name, tList *FicherosAbiertos) {
    OpenFile NewFile = (OpenFile)malloc(sizeof(struct OpenFile));
    if (NewFile == NULL)
        return false;

    NewFile->fd = fd;
    NewFile->filename = strdup(name);

    if (NewFile->filename == NULL) {
        free(NewFile);
        return false;
    }

    NewFile->flags = mode;

    return insertItem(NewFile, LNULL, FicherosAbiertos);
}

static void FormatoModo(int flags, char *buf, size_t size) {
    buf[0] = '\0';
    int acc = flags & O_ACCMODE;

    if (acc == O_RDONLY)
        strncat(buf, "O_RDONLY", size - strlen(buf) - 1);
    else if (acc == O_WRONLY)
        strncat(buf, "O_WRONLY", size - strlen(buf) - 1);
    else if (acc == O_RDWR)
        strncat(buf, "O_RDWR", size - strlen(buf) - 1);

    if (flags & O_CREAT)
        strncat(buf, " | O_CREAT", size - strlen(buf) - 1);
    if (flags & O_EXCL)
        strncat(buf, " | O_EXCL", size - strlen(buf) - 1);
    if (flags & O_APPEND)
        strncat(buf, " | O_APPEND", size - strlen(buf) - 1);
    if (flags & O_TRUNC)
        strncat(buf, " | O_TRUNC", size - strlen(buf) - 1);
}

void ListarFicherosAbiertos(tList list) {
    for (tPosL p = first(list); p != LNULL; p = next(p, list)) {
        OpenFile file = getItem(p, list);
        if (file != NULL) {
            char modostr[128];
            FormatoModo(file->flags, modostr, sizeof(modostr));
            printf("descriptor: %d -> %s %s\n", file->fd, file->filename, modostr);
        }
    }
}

void InicializarFicherosAbiertos(tList *FicherosAbiertos) {
    const char *names[] = {"entrada estandar", "salida estandar", "error estandar"};
    createEmptyList(FicherosAbiertos);
    int fds[] = {0, 1, 2};

    for (int i = 0; i < 3; i++) {
        int flags = fcntl(fds[i], F_GETFL);
        if (flags != -1) {
            AñadirAFicherosAbiertos(fds[i], flags, names[i], FicherosAbiertos);
        }
    }
}

void VaciarFicherosAbiertos(tList *FicherosAbiertos) {
    deleteListWithData(FicherosAbiertos, LiberaOpenFile);
}

/* ───────────────────── FUNCIONES AUXILIARES DE FICHEROS ───────────────────── */

static int deleteSingle(const char *name) {
    struct stat st;

    /* lstat, not stat: a symlink must be removed itself, never followed */
    if (lstat(name, &st) == -1) {
        fprintf(stderr, "Imposible borrar %s: %s\n", name, strerror(errno));
        return -1;
    }

    int rc = S_ISDIR(st.st_mode) ? rmdir(name) : unlink(name);

    if (rc == -1) {
        fprintf(stderr, "Imposible borrar %s: %s\n", name, strerror(errno));
        return -1;
    }

    return 0;
}

static int deleteTree(const char *name) {
    struct stat st;

    if (lstat(name, &st) == -1) {
        fprintf(stderr, "deltree: no se puede acceder a '%s': %s\n", name, strerror(errno));
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        if (unlink(name) == -1) {
            fprintf(stderr, "deltree: no se puede borrar '%s': %s\n", name, strerror(errno));
            return -1;
        }
        return 0;
    }

    DIR *d = opendir(name);

    if (d == NULL) {
        fprintf(stderr, "deltree: no se puede abrir '%s': %s\n", name, strerror(errno));
        return -1;
    }

    int status = 0;
    struct dirent *e;
    char path[PATH_MAX];

    while ((e = readdir(d)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;

        size_t name_len = strlen(name);
        int ret;

        if (name_len > 0 && name[name_len - 1] == '/') {
            ret = snprintf(path, sizeof(path), "%s%s", name, e->d_name);
        } else {
            ret = snprintf(path, sizeof(path), "%s/%s", name, e->d_name);
        }

        if (ret >= (int)sizeof(path)) {
            fprintf(stderr, "deltree: ruta demasiado larga: '%s/%s'\n", name, e->d_name);
            status = -1;
            continue;
        }

        if (deleteTree(path) == -1)
            status = -1;
    }

    closedir(d);

    if (rmdir(name) == -1) {
        fprintf(stderr, "deltree: no se puede borrar '%s': %s\n", name, strerror(errno));
        status = -1;
    }

    return status;
}

static void MostrarDirActual(void) {
    char dir[PATH_MAX];

    if (getcwd(dir, sizeof(dir)) == NULL)
        perror("Imposible obtener directorio");
    else
        printf("%s\n", dir);
}

static char LetraTF(mode_t m) {
    switch (m & S_IFMT) {
    case S_IFSOCK:
        return 's';
    case S_IFLNK:
        return 'l';
    case S_IFREG:
        return '-';
    case S_IFBLK:
        return 'b';
    case S_IFDIR:
        return 'd';
    case S_IFCHR:
        return 'c';
    case S_IFIFO:
        return 'p';
    default:
        return '?';
    }
}

static char *ConvierteModo(mode_t m, char *permisos) {
    strcpy(permisos, "----------");

    permisos[0] = LetraTF(m);
    if (m & S_IRUSR) permisos[1] = 'r';
    if (m & S_IWUSR) permisos[2] = 'w';
    if (m & S_IXUSR) permisos[3] = 'x';
    if (m & S_IRGRP) permisos[4] = 'r';
    if (m & S_IWGRP) permisos[5] = 'w';
    if (m & S_IXGRP) permisos[6] = 'x';
    if (m & S_IROTH) permisos[7] = 'r';
    if (m & S_IWOTH) permisos[8] = 'w';
    if (m & S_IXOTH) permisos[9] = 'x';
    if (m & S_ISUID) permisos[3] = 's';
    if (m & S_ISGID) permisos[6] = 's';
    if (m & S_ISVTX) permisos[9] = 't';

    return permisos;
}

static void PrintFileItem(const char *path, const char *name, bool long_listing, bool show_link, bool use_acc_time) {
    struct stat st;

    if (lstat(path, &st) == -1) {
        fprintf(stderr, "Error al acceder a %s: %s\n", path, strerror(errno));
        return;
    }

    if (!long_listing) {
        printf("%9ld %s", (long)st.st_size, name);
        if (show_link && S_ISLNK(st.st_mode)) {
            char link_target[PATH_MAX];
            ssize_t len = readlink(path, link_target, sizeof(link_target) - 1);
            if (len != -1) {
                link_target[len] = '\0';
                printf(" -> %s", link_target);
            }
        }
        printf("\n");
    } else {
        char permisos[16];
        ConvierteModo(st.st_mode, permisos);

        char user_str[32], group_str[32];
        struct passwd *pw = getpwuid(st.st_uid);
        if (pw != NULL) {
            snprintf(user_str, sizeof(user_str), "%s", pw->pw_name);
        } else {
            snprintf(user_str, sizeof(user_str), "%u", (unsigned int)st.st_uid);
        }

        struct group *gr = getgrgid(st.st_gid);
        if (gr != NULL) {
            snprintf(group_str, sizeof(group_str), "%s", gr->gr_name);
        } else {
            snprintf(group_str, sizeof(group_str), "%u", (unsigned int)st.st_gid);
        }

        time_t t_file = use_acc_time ? st.st_atime : st.st_mtime;
        struct tm *tm_info = localtime(&t_file);
        char timebuf[32];
        strftime(timebuf, sizeof(timebuf), "%Y/%m/%d-%H:%M", tm_info);

        printf("%s %3ld %-8s %-8s %8ld %s %s",
               permisos, (long)st.st_nlink,
               user_str, group_str,
               (long)st.st_size, timebuf, name);

        if (show_link && S_ISLNK(st.st_mode)) {
            char link_target[PATH_MAX];
            ssize_t len = readlink(path, link_target, sizeof(link_target) - 1);
            if (len != -1) {
                link_target[len] = '\0';
                printf(" -> %s", link_target);
            }
        }

        printf("\n");
    }
}

static void ListDirectoryRecursive(const char *dir_path, bool long_listing, bool show_link, bool use_acc_time, bool show_hidden, int recurse_mode) {
    DIR *d = opendir(dir_path);

    if (!d) {
        fprintf(stderr, "Error al abrir %s: %s\n", dir_path, strerror(errno));
        return;
    }

    struct dirent *entry;
    char **names = NULL;
    int count = 0;
    int capacity = 0;

    while ((entry = readdir(d)) != NULL) {
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }
        if (count >= capacity) {
            capacity = (capacity == 0) ? 32 : capacity * 2;
            char **new_names = realloc(names, capacity * sizeof(char *));
            if (!new_names) {
                perror("realloc");
                break;
            }
            names = new_names;
        }
        names[count] = strdup(entry->d_name);
        count++;
    }

    closedir(d);

    // Recursión ANTES de listar el directorio (-recb)
    if (recurse_mode == 2) {
        for (int i = 0; i < count; i++) {
            if (strcmp(names[i], ".") == 0 || strcmp(names[i], "..") == 0)
                continue;
            char subpath[PATH_MAX];
            size_t len = strlen(dir_path);
            if (len > 0 && dir_path[len - 1] == '/') {
                snprintf(subpath, sizeof(subpath), "%s%s", dir_path, names[i]);
            } else {
                snprintf(subpath, sizeof(subpath), "%s/%s", dir_path, names[i]);
            }
            struct stat st;
            if (lstat(subpath, &st) == 0 && S_ISDIR(st.st_mode)) {
                ListDirectoryRecursive(subpath, long_listing, show_link, use_acc_time, show_hidden, recurse_mode);
            }
        }
    }

    // Listar contenido del directorio actual
    printf("%s:\n", dir_path);
    for (int i = 0; i < count; i++) {
        char fullpath[PATH_MAX];
        size_t len = strlen(dir_path);
        if (len > 0 && dir_path[len - 1] == '/') {
            snprintf(fullpath, sizeof(fullpath), "%s%s", dir_path, names[i]);
        } else {
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, names[i]);
        }
        PrintFileItem(fullpath, names[i], long_listing, show_link, use_acc_time);
    }

    // Recursión DESPUÉS de listar el directorio (-reca)
    if (recurse_mode == 1) {
        for (int i = 0; i < count; i++) {
            if (strcmp(names[i], ".") == 0 || strcmp(names[i], "..") == 0)
                continue;
            char subpath[PATH_MAX];
            size_t len = strlen(dir_path);
            if (len > 0 && dir_path[len - 1] == '/') {
                snprintf(subpath, sizeof(subpath), "%s%s", dir_path, names[i]);
            } else {
                snprintf(subpath, sizeof(subpath), "%s/%s", dir_path, names[i]);
            }
            struct stat st;
            if (lstat(subpath, &st) == 0 && S_ISDIR(st.st_mode)) {
                ListDirectoryRecursive(subpath, long_listing, show_link, use_acc_time, show_hidden, recurse_mode);
            }
        }
    }

    for (int i = 0; i < count; i++) {
        free(names[i]);
    }

    free(names);
}

/* ───────────────────── COMANDOS DEL SHELL ───────────────────── */

void Cmd_date(char *tr) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (!tr) {
        char date[11];
        char hour[9];

        strftime(date, sizeof(date), "%d/%m/%Y", t);
        strftime(hour, sizeof(hour), "%H:%M:%S", t);

        printf("%s %s\n", date, hour);
    } else if (!strcmp(tr, "-d")) {
        char date[11];

        strftime(date, sizeof(date), "%d/%m/%Y", t);

        printf("%s\n", date);
    } else if (!strcmp(tr, "-t")) {
        char hour[9];

        strftime(hour, sizeof(hour), "%H:%M:%S", t);

        printf("%s\n", hour);
    } else {
        printf("Uso: date [-d|-t]\n");
    }
}

void Cmd_pid(char *arg) {
    if (arg == NULL)
        printf("El pid del proceso es %d\n", (int)getpid());
    else if (!strcmp(arg, "-p"))
        printf("El pid del proceso padre es %d\n", (int)getppid());
    else
        printf("Uso: pid [-p]\n");
}

void Cmd_autores(char *tr) {
    if (!tr)
        printf("Pablo Bea Dopazo: pablo.bea.dopazo\nSzymon Arthur Zieba Glaz: szymon.zieba\n");
    else if (!strcmp(tr, "-n"))
        printf("Pablo Bea Dopazo\nSzymon Arthur Zieba Glaz\n");
    else if (!strcmp(tr, "-l"))
        printf("pablo.bea.dopazo\nszymon.zieba\n");
    else
        printf("Uso: authors [-n|-l]\n");
}

void Cmd_sysinfo() {
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

void Cmd_help(char *tr) {
    if (!tr) {
        printf("Available commands:\n"
               "  authors [-l|-n]       date [-d|-t]          pid [-p]              sysinfo\n"
               "  chdir [dir]           help [cmd]            exit                  bye\n"
               "  open [file m1 m2...]  close [df|-f]         listopen              dup df\n"
               "  lseek df pos ref      readstr df cont       writestr df str\n"
               "  makefile name         makedir nam           delete name1...       deltree name1...\n"
               "  listfile [-long][-link][-acc] nam1...       list [-reca][-recb][-hid][-long][-link][-acc] nam1...\n");
    } else if (!strcmp(tr, "exit") || !strcmp(tr, "bye") || !strcmp(tr, "quit")) {
        printf("%s: exits the shell\n", tr);
    } else if (!strcmp(tr, "date")) {
        printf("date [-d|-t]: shows current date and/or time (-d date only, -t time only)\n");
    } else if (!strcmp(tr, "pid")) {
        printf("pid [-p]: shows the shell's pid (-p shows parent pid)\n");
    } else if (!strcmp(tr, "authors") || !strcmp(tr, "autores")) {
        printf("authors [-l|-n]: shows authors names and logins (-l logins only, -n names only)\n");
    } else if (!strcmp(tr, "sysinfo") || !strcmp(tr, "infosys")) {
        printf("sysinfo: shows information about the machine\n");
    } else if (!strcmp(tr, "help")) {
        printf("help [cmd]: shows help about commands or a specific command\n");
    } else if (!strcmp(tr, "chdir")) {
        printf("chdir [dir]: changes current working directory to dir, or prints it if no argument\n");
    } else if (!strcmp(tr, "open")) {
        printf("open [file m1 m2...]: opens file with specified modes (cr, ap, ex, ro, rw, wo, tr)\n"
               "                      without arguments lists open files\n");
    } else if (!strcmp(tr, "close")) {
        printf("close df [-f]: closes file descriptor df and eliminates list entry\n");
    } else if (!strcmp(tr, "listopen")) {
        printf("listopen: lists the shell's open files\n");
    } else if (!strcmp(tr, "dup")) {
        printf("dup df: duplicates file descriptor df and adds it to the open files list\n");
    } else if (!strcmp(tr, "lseek")) {
        printf("lseek df pos ref: positions offset (ref: SEEK_SET, SEEK_CUR, SEEK_END)\n");
    } else if (!strcmp(tr, "readstr")) {
        printf("readstr df cont: reads cont bytes from file descriptor df and prints them\n");
    } else if (!strcmp(tr, "writestr")) {
        printf("writestr df str: writes string str to open file descriptor df\n");
    } else if (!strcmp(tr, "makefile")) {
        printf("makefile name: creates an empty file\n");
    } else if (!strcmp(tr, "makedir")) {
        printf("makedir nam: creates a directory\n");
    } else if (!strcmp(tr, "delete")) {
        printf("delete name1 name2...: deletes files, links, or empty directories\n");
    } else if (!strcmp(tr, "deltree")) {
        printf("deltree name1 name2...: deletes files, links, and non-empty directories recursively\n");
    } else if (!strcmp(tr, "listfile")) {
        printf("listfile [-long][-link][-acc] nam1 nam2...: gives info on items (does not list dir contents)\n");
    } else if (!strcmp(tr, "list")) {
        printf("list [-reca][-recb][-hid][-long][-link][-acc] nam1 nam2...: lists directory contents\n");
    } else {
        printf("help: comando '%s' no encontrado\n", tr);
    }
}

void Cmd_chdir(char *dir) {
    if (dir == NULL)
        MostrarDirActual();
    else if (chdir(dir) == -1)
        perror("Imposible cambiar directorio");
}

void Cmd_open(char *tr[], tList *FicherosAbiertos) {
    if (tr[0] == NULL) {
        Cmd_listopen(FicherosAbiertos);
        return;
    }

    int mode = 0;
    bool acc_set = false;
    for (int i = 1; tr[i] != NULL; i++) {
        if (!strcmp(tr[i], "cr")) mode |= O_CREAT;
        else if (!strcmp(tr[i], "ap")) mode |= O_APPEND;
        else if (!strcmp(tr[i], "ex")) mode |= O_EXCL;
        else if (!strcmp(tr[i], "tr")) mode |= O_TRUNC;
        else if (!strcmp(tr[i], "ro")) {
            mode |= O_RDONLY;
            acc_set = true;
        } else if (!strcmp(tr[i], "wo")) {
            mode |= O_WRONLY;
            acc_set = true;
        } else if (!strcmp(tr[i], "rw")) {
            mode |= O_RDWR;
            acc_set = true;
        } else {
            fprintf(stderr, "open: modo desconocido '%s'\n", tr[i]);
        }
    }

    if (!acc_set) {
        mode |= O_RDWR;
    }

    int df = open(tr[0], mode, 0777);
    if (df == -1) {
        perror("Imposible abrir fichero");
    } else {
        if (AñadirAFicherosAbiertos(df, mode, tr[0], FicherosAbiertos)) {
            char modostr[128];
            FormatoModo(mode, modostr, sizeof(modostr));
            printf("Anadida entrada a la tabla ficheros abiertos: descriptor-> %d, modo-> %s, nombre-> %s\n", df, modostr, tr[0]);
        } else {
            printf("No se pudo añadir a la lista\n");
        }
    }
}

void Cmd_close(char *tr[], tList *FicherosAbiertos) {
    if (tr[0] == NULL || tr[2] != NULL) {
        printf("Uso: close df [-f]\n");
        return;
    }

    int df = -1;
    bool force = false;

    if (tr[1] == NULL) {
        if (!strcmp(tr[0], "-f")) {
            printf("Uso: close df [-f]\n");
            return;
        }
        df = atoi(tr[0]);
    } else {
        if (!strcmp(tr[1], "-f") && strcmp(tr[0], "-f") != 0) {
            df = atoi(tr[0]);
            force = true;
        } else if (!strcmp(tr[0], "-f") && strcmp(tr[1], "-f") != 0) {
            df = atoi(tr[1]);
            force = true;
        } else {
            printf("Uso: close df [-f]\n");
            return;
        }
    }

    (void)force; /* En P1 no hay mapeos activos de memoria (mmap se introduce en P2);
                    el flag -f se procesa para cumplir la sintaxis del PDF y permitir
                    cerrar aun cuando se añada soporte de mapeos en P2 */

    if (close(df) == -1) {
        perror("Imposible cerrar descriptor");
        return;
    }

    tPosL pos = findItemByFileDescriptor(df, *FicherosAbiertos);

    if (pos != LNULL) {
        OpenFile of = getItem(pos, *FicherosAbiertos);
        LiberaOpenFile(of);
        deleteAtPosition(pos, FicherosAbiertos);
    } else {
        printf("Descriptor %d cerrado (no estaba en la lista de ficheros abiertos)\n", df);
    }
}

void Cmd_listopen(tList *FicherosAbiertos) {
    if (FicherosAbiertos != NULL) {
        ListarFicherosAbiertos(*FicherosAbiertos);
    }
}

void Cmd_dup(char *tr[], tList *FicherosAbiertos) {
    if (tr[0] == NULL) {
        printf("Uso: dup df\n");
        return;
    }

    int oldfd = atoi(tr[0]);
    int newfd = dup(oldfd);
    if (newfd == -1) {
        perror("Imposible duplicar descriptor");
        return;
    }

    char dup_name[512];
    int mode = fcntl(newfd, F_GETFL);
    if (mode == -1) mode = 0;

    tPosL pos = findItemByFileDescriptor(oldfd, *FicherosAbiertos);
    if (pos != LNULL) {
        OpenFile oldFile = getItem(pos, *FicherosAbiertos);
        snprintf(dup_name, sizeof(dup_name), "dup %d (%s)", oldfd, oldFile->filename);
    } else {
        snprintf(dup_name, sizeof(dup_name), "dup %d", oldfd);
    }

    AñadirAFicherosAbiertos(newfd, mode, dup_name, FicherosAbiertos);
    printf("Descriptor %d duplicado en %d\n", oldfd, newfd);
}

void Cmd_lseek(char *tr[]) {
    if (tr[0] == NULL || tr[1] == NULL || tr[2] == NULL) {
        printf("Uso: lseek df pos ref\n"
               "ref puede ser SEEK_SET, SEEK_CUR o SEEK_END\n");
        return;
    }

    int df = atoi(tr[0]);
    off_t pos = atoll(tr[1]);
    int whence;

    if (!strcmp(tr[2], "SEEK_SET"))
        whence = SEEK_SET;
    else if (!strcmp(tr[2], "SEEK_CUR"))
        whence = SEEK_CUR;
    else if (!strcmp(tr[2], "SEEK_END"))
        whence = SEEK_END;
    else {
        printf("Referencia no valida: %s (debe ser SEEK_SET, SEEK_CUR o SEEK_END)\n", tr[2]);
        return;
    }

    off_t newpos = lseek(df, pos, whence);

    if (newpos == (off_t)-1) {
        perror("Imposible posicionar puntero");
    } else {
        printf("Nueva posicion del descriptor %d: %lld\n", df, (long long)newpos);
    }
}

void Cmd_readstr(char *tr[]) {
    if (tr[0] == NULL || tr[1] == NULL) {
        printf("Uso: readstr df cont\n");
        return;
    }

    int df = atoi(tr[0]);
    long cont = atol(tr[1]);

    if (cont <= 0) {
        printf("El numero de bytes debe ser mayor que 0\n");
        return;
    }

    char *buffer = malloc(cont + 1);
    if (!buffer) {
        perror("malloc");
        return;
    }

    ssize_t status = read(df, buffer, cont);

    if (status == -1) {
        perror("Imposible leer de descriptor");
    } else {
        buffer[status] = '\0';
        printf("%s\n", buffer);
    }

    free(buffer);
}

void Cmd_writestr(char *tr[]) {
    if (tr[0] == NULL || tr[1] == NULL) {
        printf("Uso: writestr df str\n");
        return;
    }

    int df = atoi(tr[0]);
    size_t total_len = 0;

    for (int i = 1; tr[i] != NULL; i++) {
        total_len += strlen(tr[i]) + 1;
    }

    char *buffer = malloc(total_len + 1);

    if (!buffer) {
        perror("malloc");
        return;
    }
    
    buffer[0] = '\0';

    for (int i = 1; tr[i] != NULL; i++) {
        strcat(buffer, tr[i]);
        if (tr[i + 1] != NULL) strcat(buffer, " ");
    }

    ssize_t written = write(df, buffer, strlen(buffer));

    if (written == -1) {
        perror("Imposible escribir en descriptor");
    } else {
        printf("%zd bytes escritos en el descriptor %d\n", written, df);
    }

    free(buffer);
}

int Cmd_makefile(char *tr) {
    if (tr == NULL) {
        printf("Uso: makefile name\n");
        return 1;
    }

    if (open(tr, O_CREAT | O_TRUNC) == -1) {
        perror("Imposible crear fichero");
        return 1;
    }

    return 0;
}

int Cmd_makedir(char *tr) {
    if (tr == NULL) {
        printf("Uso: makedir nam\n");
        return 1;
    }

    if (mkdir(tr, O_CREAT) == -1) {
        perror("Imposible crear directorio");
        return 1;
    }

    return 0;
}

void Cmd_delete(char *tr[]) {
    if (tr[0] == NULL) {
        printf("Uso: delete name1 name2 ...\n");
        return;
    }

    for (int i = 0; tr[i] != NULL; i++) {
        deleteSingle(tr[i]);
    }
}

void Cmd_deltree(char *tr[]) {
    if (tr[0] == NULL) {
        printf("Uso: deltree name1 name2 ...\n");
        return;
    }

    for (int i = 0; tr[i] != NULL; i++) {
        deleteTree(tr[i]);
    }
}

void Cmd_listfile(char *tr[]) {
    bool long_listing = false;
    bool show_link = false;
    bool use_acc_time = false;

    int num_files = 0;

    for (int i = 0; tr[i] != NULL; i++) {
        if (!strcmp(tr[i], "-long")) {
            long_listing = true;
        } else if (!strcmp(tr[i], "-link")) {
            show_link = true;
        } else if (!strcmp(tr[i], "-acc")) {
            use_acc_time = true;
        } else {
            num_files++;
        }
    }

    if (num_files == 0) {
        PrintFileItem(".", ".", long_listing, show_link, use_acc_time);
        return;
    }

    for (int i = 0; tr[i] != NULL; i++) {
        if (!strcmp(tr[i], "-long") || !strcmp(tr[i], "-link") || !strcmp(tr[i], "-acc")) {
            continue;
        }
        PrintFileItem(tr[i], tr[i], long_listing, show_link, use_acc_time);
    }
}

void Cmd_list(char *tr[]) {
    bool long_listing = false;
    bool show_link = false;
    bool use_acc_time = false;
    bool show_hidden = false;
    int recurse_mode = 0; // 0: none, 1: reca, 2: recb

    int num_files = 0;

    for (int i = 0; tr[i] != NULL; i++) {
        if (!strcmp(tr[i], "-long")) {
            long_listing = true;
        } else if (!strcmp(tr[i], "-link")) {
            show_link = true;
        } else if (!strcmp(tr[i], "-acc")) {
            use_acc_time = true;
        } else if (!strcmp(tr[i], "-hid")) {
            show_hidden = true;
        } else if (!strcmp(tr[i], "-reca")) {
            recurse_mode = 1;
        } else if (!strcmp(tr[i], "-recb")) {
            recurse_mode = 2;
        } else {
            num_files++;
        }
    }

    if (num_files == 0) {
        ListDirectoryRecursive(".", long_listing, show_link, use_acc_time, show_hidden, recurse_mode);
        return;
    }

    for (int i = 0; tr[i] != NULL; i++) {
        if (!strcmp(tr[i], "-long") || !strcmp(tr[i], "-link") || !strcmp(tr[i], "-acc") ||
            !strcmp(tr[i], "-hid") || !strcmp(tr[i], "-reca") || !strcmp(tr[i], "-recb")) {
            continue;
        }

        struct stat st;

        if (lstat(tr[i], &st) == -1) {
            fprintf(stderr, "Error al acceder a %s: %s\n", tr[i], strerror(errno));
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            ListDirectoryRecursive(tr[i], long_listing, show_link, use_acc_time, show_hidden, recurse_mode);
        } else {
            PrintFileItem(tr[i], tr[i], long_listing, show_link, use_acc_time);
        }
    }
}
