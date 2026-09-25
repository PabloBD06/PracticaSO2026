// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comandosp1.c"
#include "dynamicList.c"

#define MAXENTRADA 2048
#define MAXOPENFILES 128

tList FicherosAbiertos;

/**************************SHELL**************************/
void DecidirComando(char *tr[]) {
    if (tr[0] == NULL) /*por si cambiamos lo de TroearCadena==0*/
        return;        /*no hace falta que ya comprobamos que TrocearCaPontevedradena no devielve 0*/
    if (!strcmp(tr[0], "quit") || !strcmp(tr[0], "exit")) exit(0);
    else if (!strcmp(tr[0], "date")) Cmd_date(tr[1]);
    else if (!strcmp(tr[0], "pid")) Cmd_pid(tr[1]);
    else if (!strcmp(tr[0], "authors")) Cmd_autores(tr[1]);
    else if (!strcmp(tr[0], "sysinfo")) Cmd_sysinfo();
    else if (!strcmp(tr[0], "help")) Cmd_help(tr[1]);
    else if (!strcmp(tr[0], "chdir")) Cmd_chdir(tr[1]);
    else if (!strcmp(tr[0], "open")) Cmd_open(&tr[1]);
    else if (!strcmp(tr[0], "close")) Cmd_close();
    else if (!strcmp(tr[0], "listopen")) Cmd_listopen();
    else if (!strcmp(tr[0], "dup")) Cmd_dup();
    else if (!strcmp(tr[0], "lseek")) Cmd_lseek();
    else if (!strcmp(tr[0], "readstr")) Cmd_readstr();
    else if (!strcmp(tr[0], "writestr")) Cmd_writestr();
    else if (!strcmp(tr[0], "makefile")) Cmd_makefile(tr[1]);
    else if (!strcmp(tr[0], "makedir")) Cmd_makedir(tr[1]);
    else if (!strcmp(tr[0], "delete")) Cmd_delete();
    else if (!strcmp(tr[0], "deltree")) Cmd_deltree();
    else if (!strcmp(tr[0], "listfile")) Cmd_listfile();
    else if (!strcmp(tr[0], "list")) Cmd_list();
    else
        ;
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
    if (TrocearCadena(entrada, tr) == 0) /*no hay nada*/
        return;
    DecidirComando(tr);
}

int main(int argc, char *argv[], char *ent[]) {
    char entrada[MAXENTRADA];
    tList *FicherosAbiertos = malloc(sizeof(tList));
    createEmptyList(FicherosAbiertos);
    while (1) {
        printf("-> ");
        fgets(entrada, MAXENTRADA, stdin);
        ProcesarEntrada(entrada);
    }
}