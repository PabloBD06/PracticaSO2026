// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shellCommands.h"
#include "memoryCommands.h"
#include "dynamicList.h"

#define MAXENTRADA 2048

tList FicherosAbiertos;
tList MemoriaAlocada;

/**************************SHELL**************************/
void DecidirComando(char *tr[]) {
    if (tr[0] == NULL)
        return;
    if (!strcmp(tr[0], "quit") || !strcmp(tr[0], "exit") || !strcmp(tr[0], "bye")) {
        VaciarFicherosAbiertos(&FicherosAbiertos);
        VaciarMemoria(&MemoriaAlocada);
        exit(0);
    } else if (!strcmp(tr[0], "date")) Cmd_date(tr[1]);
    else if (!strcmp(tr[0], "pid")) Cmd_pid(tr[1]);
    else if (!strcmp(tr[0], "authors")) Cmd_autores(tr[1]);
    else if (!strcmp(tr[0], "sysinfo")) Cmd_sysinfo();
    else if (!strcmp(tr[0], "help")) Cmd_help(tr[1]);
    else if (!strcmp(tr[0], "pwd")) Cmd_chdir(NULL);
    else if (!strcmp(tr[0], "chdir")) Cmd_chdir(tr[1]);
    else if (!strcmp(tr[0], "open")) Cmd_open(&tr[1], &FicherosAbiertos);
    else if (!strcmp(tr[0], "close")) Cmd_close(&tr[1], &FicherosAbiertos);
    else if (!strcmp(tr[0], "listopen")) Cmd_listopen(&FicherosAbiertos);
    else if (!strcmp(tr[0], "dup")) Cmd_dup(&tr[1], &FicherosAbiertos);
    else if (!strcmp(tr[0], "lseek")) Cmd_lseek(&tr[1]);
    else if (!strcmp(tr[0], "readstr")) Cmd_readstr(&tr[1]);
    else if (!strcmp(tr[0], "writestr")) Cmd_writestr(&tr[1]);
    else if (!strcmp(tr[0], "makefile")) Cmd_makefile(tr[1]);
    else if (!strcmp(tr[0], "makedir")) Cmd_makedir(tr[1]);
    else if (!strcmp(tr[0], "delete")) Cmd_delete(&tr[1]);
    else if (!strcmp(tr[0], "deltree")) Cmd_deltree(&tr[1]);
    else if (!strcmp(tr[0], "listfile")) Cmd_listfile(&tr[1]);
    else if (!strcmp(tr[0], "list")) Cmd_list(&tr[1]);
    else if (!strcmp(tr[0], "allocate")) Cmd_allocate(&tr[1], &MemoriaAlocada);
    else if (!strcmp(tr[0], "deallocate")) Cmd_deallocate();
    else if (!strcmp(tr[0], "i-o")) Cmd_io();
    else if (!strcmp(tr[0], "memory")) Cmd_memory();
    else if (!strcmp(tr[0], "read")) Cmd_write();
    else if (!strcmp(tr[0], "write")) Cmd_read();
    else if (!strcmp(tr[0], "recurse")) Cmd_recurse();
    else if (!strcmp(tr[0], "memfill")) Cmd_memfill();
    else if (!strcmp(tr[0], "memdump")) Cmd_memdump();
    else
        printf("Comando no reconocido: %s (usa 'help' para ver la lista de comandos)\n", tr[0]);
}

int TrocearCadena(char *cadena, char *trozos[]) {
    int i = 1;
    if ((trozos[0] = strtok(cadena, " \n\t")) == NULL)
        return 0;
    while ((trozos[i] = strtok(NULL, " \n\t")) != NULL)
        i++;
    return i;
}

void ProcesarEntrada(char *entrada) {
    char *tr[MAXENTRADA / 2];
    if (TrocearCadena(entrada, tr) == 0)
        return;
    DecidirComando(tr);
}

int main(int argc, char *argv[], char *ent[]) {
    (void)argc;
    (void)argv;
    (void)ent;
    char entrada[MAXENTRADA];

    InicializarFicherosAbiertos(&FicherosAbiertos);
    createEmptyList(&MemoriaAlocada);

    while (1) {
        printf("-> ");
        fgets(entrada, MAXENTRADA, stdin);
        ProcesarEntrada(entrada);
    }

    VaciarFicherosAbiertos(&FicherosAbiertos);
    VaciarMemoria(&MemoriaAlocada);

    return 0;
}