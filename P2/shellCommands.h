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
    bool IsMapped;  // Marks if a file is maped by the mmmap comand
} *OpenFile;        // note: now a struct type, not a pointer typedef

void InicializarFicherosAbiertos(tList *);
void VaciarFicherosAbiertos(tList *);
void VaciarMemoria(tList *);
int matchByFd(const void *, const void *);
void LiberaOpenFile(void *);
bool AñadirAFicherosAbiertos(int, int, const char *, tList *, bool);

void Cmd_date(char *);
void Cmd_pid(char *);
void Cmd_autores(char *);
void Cmd_sysinfo(void);
void Cmd_help(char *);
void Cmd_chdir(char *);
void Cmd_open(char *[], tList *);
void Cmd_close(char *[], tList *);
void Cmd_listopen(tList *);
void Cmd_dup(char *[], tList *);
void Cmd_lseek(char *[]);
void Cmd_readstr(char *[]);
void Cmd_writestr(char *[]);
int Cmd_makefile(char *);
int Cmd_makedir(char *);
void Cmd_delete(char *[]);
void Cmd_deltree(char *[]);
void Cmd_listfile(char *[]);
void Cmd_list(char *[]);

#endif