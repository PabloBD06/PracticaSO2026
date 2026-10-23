// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

#ifndef COMANDOS_H
#define COMANDOS_H

#include "dynamicList.h"

void InicializarFicherosAbiertos(tList *FicherosAbiertos);
void VaciarFicherosAbiertos(tList *FicherosAbiertos);
void VaciarMemoria(tList *MemoriaAlocada);

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
void Cmd_allocate ();
void Cmd_deallocate ();
void Cmd_io ();
void Cmd_memory ();
void Cmd_read ();
void Cmd_write ();
void Cmd_recurse ();
void Cmd_memfill ();
void Cmd_memdump ();

#endif