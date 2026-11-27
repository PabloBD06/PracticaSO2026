// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

#ifndef PROCESSCOMMANDS_H
#define PROCESSCOMMANDS_H

#include "dynamicList.h"
#include <time.h>

enum processType {
    FINISHED,
    STOPPED,
    SIGNALED,
    ACTIVE
};

typedef struct {
    int pid;
    time_t date;
    enum processType status;
    char** commandLine;
} process;

void Cmd_getuid();
void Cmd_setuid(char *tr[]);
void Cmd_prio(char *tr[]);
void Cmd_path(char *tr[]);
void CleanupSearchPath(void);
void SetMainEnvironment(char **environment, char ***environmentStorage);
void Cmd_showenv(char *tr[]);
void Cmd_showvar(char *tr[]);
void Cmd_changevar(char *tr[]);
void Cmd_fork(char *tr[]);
void Cmd_ejec(char *tr[]);
void Cmd_ejecpri(char *tr[]);
void Cmd_pplano(char *tr[]);
void Cmd_pplanopri(char *tr[]);
void Cmd_splano(char *tr[]);
void Cmd_splanopri(char *tr[]);
void Cmd_jobs(char *tr[]);
void Cmd_deljobs(char *tr[]);

#endif
