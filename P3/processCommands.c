#include "processCommands.h"
#include <errno.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

extern char **environ;

static char **MainEnvironment;
static char ***MainEnvironmentStorage;

void SetMainEnvironment(char **environment, char ***environmentStorage) {
    MainEnvironment = environment;
    MainEnvironmentStorage = environmentStorage;
}

static void PrintEnvironment(char **environment) {
    int i;
    if (environment == NULL)
        return;
    for (i = 0; environment[i] != NULL; i++)
        puts(environment[i]);
}

static void PrintEnvironmentAddresses(void) {
    int i;

    if (MainEnvironment != NULL) {
        for (i = 0; MainEnvironment[i] != NULL; i++)
            printf("%p->main arg3[%d]=(%p) %s\n",
                   (void *)&MainEnvironment[i], i,
                   (void *)MainEnvironment[i], MainEnvironment[i]);
    }
    printf("environ:   %p (almacenado en %p)\n",
           (void *)environ, (void *)&environ);
    printf("main arg3: %p (almacenado en %p)\n",
           (void *)MainEnvironment, (void *)MainEnvironmentStorage);
}

/* Adaptado de P3/HelpCodes/Shell/Version-4/path.c: las funciones siguientes
 * conservan su lista de directorios y las operaciones usadas por Cmd_path. */
static tList SearchPath;

static int ComparePathDirectory(const void *left, const void *right) {
    return strcmp((const char *)left, (const char *)right) == 0;
}

static int PathAdd(char *directory) {
    char *copy;

    if (findItemFunc(directory, SearchPath, ComparePathDirectory) != LNULL)
        return 0;
    copy = strdup(directory);
    if (copy == NULL)
        return -1;
    if (!insertItem(copy, LNULL, &SearchPath)) {
        free(copy);
        return -1;
    }
    return 0;
}

static void PathClear(void) {
    deleteListWithData(&SearchPath, free);
}

static int PathDel(char *directory) {
    tPosL position = findItemFunc(directory, SearchPath, ComparePathDirectory);
    if (position == LNULL) {
        errno = ENOENT;
        return -1;
    }
    free(getItem(position, SearchPath));
    deleteAtPosition(position, &SearchPath);
    return 0;
}

static void PathPrint(void) {
    tPosL position;
    for (position = first(SearchPath); position != LNULL;
         position = next(position, SearchPath))
        printf("%s\n", (char *)getItem(position, SearchPath));
}

static int PathAddPath(void) {
    char *environmentPath = getenv("PATH");
    char *directory;
    char *copy;
    int imported = 0;

    if (environmentPath == NULL)
        return 0;
    copy = strdup(environmentPath);
    if (copy == NULL)
        return -1;
    for (directory = strtok(copy, ":"); directory != NULL;
         directory = strtok(NULL, ":")) {
        if (PathAdd(directory) == -1) {
            free(copy);
            return -1;
        }
        imported++;
    }
    free(copy);
    return imported;
}

void CleanupSearchPath(void) {
    PathClear();
}

void Cmd_getuid() {
    uid_t uid = getuid();
    struct passwd *usuario = getpwuid(uid);
    printf("Credencial real: %d (%s)\n", uid, usuario ? usuario->pw_name : "Desconocido");
    uid_t euid = geteuid();
    usuario = getpwuid(euid);
    printf("Credencial efectiva: %d (%s)\n", euid, usuario ? usuario->pw_name : "Desconocido");
}

void Cmd_setuid(char *tr[]) {
    // Si no hay argumentos se imprimen las credenciales
    if (tr[0] == NULL) {
        Cmd_getuid();
        return;
    }
    if (strcmp(tr[0], "-l") == 0) {
        if (tr[1] == NULL) {
            Cmd_getuid();
            return;
        }
        char *login = tr[1];
        struct passwd *usuario = getpwnam(login);
        if (usuario == NULL) {
            perror("getpwnam");
            return;
        }
        if (seteuid(usuario->pw_uid) == -1) {
            perror("seteuid");
            return;
        }
        return;
    } else {
        uid_t uid = atoi(tr[0]);
        if (seteuid(uid) == -1) {
            perror("seteuid");
            return;
        }
        return;
    }
}

void Cmd_prio(char *tr[]) {
    errno = 0;
    pid_t pid;
    if (tr[0] == NULL) {
        pid = getpid();
    } else {
        pid = atoi(tr[0]);
    }
    if (tr[1] == NULL) {
        int prio = getpriority(PRIO_PROCESS, pid);
        if (prio == -1 && errno != 0) {
            perror("getpriority");
            return;
        }
        printf("Prioridad del proceso %d es %d\n", pid, prio);
    } else {
        int new_prio = atoi(tr[1]);
        if (setpriority(PRIO_PROCESS, pid, new_prio) == -1) {
            perror("setpriority");
            return;
        }
        printf("Prioridad del proceso %d cambiada a: %d\n", pid, new_prio);
    }
}

/* PathAdd/PathDel/PathClear/PathPrint/PathAddPath vienen de
 * P3/HelpCodes/Shell/Version-4/path.c; el despacho sigue la sintaxis de P3. */
void Cmd_path(char *tr[]) {
    if (tr[0] == NULL) {
        PathPrint();
        return;
    }

    if (strcmp(tr[0], "-add") == 0) {
        if (tr[1] == NULL || tr[2] != NULL) {
            fprintf(stderr, "Uso: path -add directorio\n");
            return;
        }
        if (PathAdd(tr[1]) == -1)
            perror("path -add");
        return;
    }

    if (strcmp(tr[0], "-del") == 0) {
        if (tr[1] == NULL || tr[2] != NULL) {
            fprintf(stderr, "Uso: path -del directorio\n");
            return;
        }
        if (PathDel(tr[1]) == -1)
            perror("path -del");
        return;
    }

    if (strcmp(tr[0], "-clear") == 0) {
        if (tr[1] != NULL) {
            fprintf(stderr, "Uso: path -clear\n");
            return;
        }
        PathClear();
        return;
    }

    if (strcmp(tr[0], "-path") == 0) {
        int imported;
        if (tr[1] != NULL) {
            fprintf(stderr, "Uso: path -path\n");
            return;
        }
        imported = PathAddPath();
        if (imported == -1) {
            perror("path -path");
            return;
        }
        printf("Importados %d directorios en la ruta de busqueda\n", imported);
        return;
    }

    fprintf(stderr, "Uso: path [-add directorio|-del directorio|-clear|-path]\n");
}

void Cmd_showenv(char *tr[]) {
    if (tr[0] == NULL) {
        PrintEnvironment(MainEnvironment);
    } else if (strcmp(tr[0], "-environ") == 0 && tr[1] == NULL) {
        PrintEnvironment(environ);
    } else if (strcmp(tr[0], "-addr") == 0 && tr[1] == NULL) {
        PrintEnvironmentAddresses();
    } else {
        fprintf(stderr, "Uso: showenv [-environ|-addr]\n");
    }
}
void Cmd_showvar(char *tr[]) {}
void Cmd_changevar(char *tr[]) {}
void Cmd_fork(char *tr[]) {}
void Cmd_ejec(char *tr[]) {}
void Cmd_ejecpri(char *tr[]) {}
void Cmd_pplano(char *tr[]) {}
void Cmd_pplanopri(char *tr[]) {}
void Cmd_splano(char *tr[]) {}
void Cmd_splanopri(char *tr[]) {}
void Cmd_jobs(char *tr[]) {}
void Cmd_deljobs(char *tr[]) {}
