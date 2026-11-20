// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

#ifndef MEMORYCOMMANDS_H
#define MEMORYCOMMANDS_H

#include "dynamicList.h"

void InicializarFicherosAbiertos(tList *);
void VaciarFicherosAbiertos(tList *);
void VaciarMemoria(tList *);

int Cmd_allocate(char *[], tList *, tList *);
void Cmd_deallocate(char *[], tList *, tList *);
void Cmd_io(char *[]);
int Cmd_memory(char *[], tList *);
void Cmd_read(char *[]);
void Cmd_write(char *[]);
int Cmd_recurse(char *[]);
int Cmd_memfill(char *[]);
int Cmd_memdump(char *[]);
#endif