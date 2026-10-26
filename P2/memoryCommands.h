// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

#ifndef MEMORYCOMMANDS_H
#define MEMORYCOMMANDS_H

#include "dynamicList.h"

void InicializarFicherosAbiertos(tList *FicherosAbiertos);
void VaciarFicherosAbiertos(tList *FicherosAbiertos);
void VaciarMemoria(tList *MemoriaAlocada);

int Cmd_allocate(char *tr[], tList *MemoriaAlocada);
void Cmd_deallocate();
void Cmd_io();
void Cmd_memory();
void Cmd_read();
void Cmd_write();
void Cmd_recurse();
void Cmd_memfill();
void Cmd_memdump();

#endif

