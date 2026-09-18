// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXENTRADA 2048
#define MAXNOMBRE 1024

void MostrarDirActual() {
    char dir[MAXNOMBRE];

    if (getcwd(dir, MAXNOMBRE) == NULL)
        perror("Imposible obtener directorio");
    else
        printf("%s\n", dir);
}

int ComprobarSegundoPlano(char *tr[]) {
    int i;
    for (i = 0; tr[i] != NULL; i++)
        if (!strcmp(tr[i], "&")) { /*& indica segundo plano*/
            tr[i] = NULL;          /*es el ultimo argumento*/
            return i;              /*si solo hay un & no se ejecuta nada en pplano*/
        }
    return 0;
}

void Proceso(char *tr[], int splano) {
    pid_t pid;
    void Cmd_exec(char **);
    int background = splano || ComprobarSegundoPlano(tr);
    if ((pid = fork()) == -1) {
        perror("Imposible crear proceso");
        return;
    }
    if (pid == 0) { /*proceso hijo*/
        Cmd_exec(tr);
        exit(255); /*por si falla exec*/
    }
    if (!background)
        waitpid(pid, NULL, 0);
}

/*********************************************/
/*************COMANDOS DEL SHELL************************/

void Cmd_autores() {
    printf("Los autores del shell....\n");
}

void Cmd_exec(char *tr[]) {
    if (execvp(tr[0], tr) == -1)
        perror("Imposible ejecutar");
}

void Cmd_splano(char *arg[]) {
    Proceso(arg, 1);
}
void Cmd_pplano(char *arg[]) {
    Proceso(arg, 0);
}

void Cmd_chdir(char *dir) {
    if (dir == NULL)
        MostrarDirActual();
    else if (chdir(dir) == -1)
        perror("Imposible cambiar directorio");
}

void Cmd_pwd() {
    MostrarDirActual();
}

void Cmd_pid(char *arg) {
    if (arg == NULL)
        printf("El pid del proceso es %d\n", (int)getpid());
    else if (!strcmp(arg, "-p"))
        printf("El pid del proceso padre es %d\n", (int)getppid());
}

/**************************SHELL**************************/
void DecidirComando(char *tr[]) {
    if (tr[0] == NULL) /*por si cambiamos lo de TroearCadena==0*/
        return;        /*no hace falta que ya comprobamos que TrocearCadena no devielve 0*/
    if (!strcmp(tr[0], "fin"))
        exit(0);
    else if (!strcmp(tr[0], "quit"))
        exit(0);
    else if (!strcmp(tr[0], "exit"))
        exit(0);
    else if (!strcmp(tr[0], "autores"))
        Cmd_autores();
    else if (!strcmp(tr[0], "exec"))
        Cmd_exec(tr + 1);
    else if (!strcmp(tr[0], "pplano"))
        Cmd_pplano(tr + 1);
    else if (!strcmp(tr[0], "splano"))
        Cmd_splano(tr + 1);
    else if (!strcmp(tr[0], "chdir"))
        Cmd_chdir(tr[1]);
    else if (!strcmp(tr[0], "pwd"))
        Cmd_pwd();
    else if (!strcmp(tr[0], "pid"))
        Cmd_pid(tr[1]);
    else
        Cmd_pplano(tr);
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

    while (1) {
        printf("-> ");
        fgets(entrada, MAXENTRADA, stdin);
        ProcesarEntrada(entrada);
    }
}