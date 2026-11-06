// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

#ifndef SHELLCOMMANDS_H
#define SHELLCOMMANDS_H

#include "dynamicList.h"
#include <stdbool.h>

typedef struct OpenFile {
    int fd;         // file descriptor
    char *filename; // file name
    int flags;      // flags the file was opened with
    bool IsMapped;   // Marks if a file is maped by the mmmap comand
} *OpenFile;        // note: now a struct type, not a pointer typedef

void InicializarFicherosAbiertos(tList *FicherosAbiertos);
void VaciarFicherosAbiertos(tList *FicherosAbiertos);
void VaciarMemoria(tList *MemoriaAlocada);
int matchByFd(const void *item, const void *key);
void LiberaOpenFile(void *ptr);
bool AñadirAFicherosAbiertos(int fd, int mode, const char *name, tList *FicherosAbiertos, bool IsMapped);


void Cmd_date(char *tr);
void Cmd_pid(char *arg);
void Cmd_autores(char *tr);
void Cmd_sysinfo(void);
void Cmd_help(char *tr);
void Cmd_chdir(char *dir);
void Cmd_open(char *tr[], tList *FicherosAbiertos);
void Cmd_close(char *tr[], tList *FicherosAbiertos);
void Cmd_listopen(tList *FicherosAbiertos);
void Cmd_dup(char *tr[], tList *FicherosAbiertos);
void Cmd_lseek(char *tr[]);
void Cmd_readstr(char *tr[]);
void Cmd_writestr(char *tr[]);
int Cmd_makefile(char *tr);
int Cmd_makedir(char *tr);
void Cmd_delete(char *tr[]);
void Cmd_deltree(char *tr[]);
void Cmd_listfile(char *tr[]);
void Cmd_list(char *tr[]);

#endif