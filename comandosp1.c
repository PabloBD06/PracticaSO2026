#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

#include "comandosp1.h"
#include "dynamicList.h"

#define MAXNOMBREDIR 256 // tamaño maximo de nombre de directorio

void ListarFicherosAbiertos(tList list) {
    if (!isEmptyList(list)) {
        tPosL currentpos = first(list);
        do {
            OpenFile file = getItem(currentpos, list);
            printf("descriptor: %d,-> %s\n", file->fd, file->filename);
            currentpos = next(currentpos, list);

        } while (next(currentpos, list) != NULL);
    }
}

bool AñadirAFicherosAbiertos(int fd, int mode, char *name, tList *FicherosAbiertos){
    OpenFile NewFile = (OpenFile)malloc(sizeof(OpenFile));
        NewFile->fd = fd;
        NewFile->filename = name;
        NewFile->flags = mode;
    return insertItem(NewFile, LNULL, FicherosAbiertos);

}

/**
 * Función auxiliar usada en Cmd_chdir
 */
void MostrarDirActual() {
    char dir[MAXNOMBREDIR];

    if (getcwd(dir, MAXNOMBREDIR) == NULL)
        perror("Imposible obtener directorio");
    else
        printf("%s\n", dir);
}

void Cmd_date(char *tr) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (!tr) {
        char date[21];
        strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", t);
        printf("%s\n", date);
    } else if (!strcmp(tr, "-d")) {
        char date[11];
        strftime(date, sizeof(date), "%Y-%m-%d", t);
        printf("%s\n", date);
    } else if (!strcmp(tr, "-t")) {
        char hour[9];
        strftime(hour, sizeof(hour), "%H:%M:%S", t);
        printf("%s\n", hour);
    }
}

void Cmd_pid(char *arg) {
    if (arg == NULL)
        printf("El pid del proceso es %d\n", (int)getpid());
    else if (!strcmp(arg, "-p"))
        printf("El pid del proceso padre es %d\n", (int)getppid());
}

void Cmd_autores(char *tr) {
    if (!tr)
        printf("Pablo Bea Dopazo(pablo.bea.dopazo)\nSzymon Artur Zieba Glaz(szymon.zieba)\n");
    else if (strcmp(tr, "-n"))
        printf("Pablo Bea Dopazo\nSzymon Artur Zieba Glaz\n");
    else if (strcmp(tr, "-l"))
        printf("pablo.bea.dopazo\nszymon.zieba\n");
}

void Cmd_sysinfo() {
    struct utsname info;

    if (uname(&info) == -1) {
        perror("uname");
        return;
    }

    printf("Sistema operativo: %s\n", info.sysname);
    printf("Nombre de la máquina: %s\n", info.nodename);
    printf("Release/kernel: %s\n", info.release);
    printf("Versión del kernel: %s\n", info.version);
    printf("Arquitectura: %s\n", info.machine);
}

void Cmd_help(char *tr) {
    if (!tr) {
        printf("Cmds practica 0: prompt numserie limits systemd command historic\n"
               "Cmds practica 1: exit bye date pid authors sysinfo help chdir open close listopen dup lseek readstr writestr makefile makedir delete deltree listfile list\n"
               "Cmds practica 2: allocate deallocate i-o read write memory recurse memfill memdump\n"
               "Cmds practica 3: getuid setuid prio path showenv showvar changevar changevarWrong fork ejec ejecpri pplano pplanopri splano splanopri jobs deljobs\n");
    } else if (strcmp(tr, "exit") || strcmp(tr, "bye"))
        printf("Termina la ejecucion del shell");
    else if (strcmp(tr, "date"))
        printf("date: [-d|-t]	Muestra la fecha y/o la hora actual");
    else if (strcmp(tr, "pid"))
        printf("pid: [-p]	Muestra el pid del shell o de su proceso padre");
    else if (strcmp(tr, "authors"))
        printf("authors: [-n|-l]	Muestra los nombres y/o logins de los autores");
    else if (strcmp(tr, "sysinfo"))
        printf("sysinfo: 	Muestra informacion de la maquina donde corre el shell");
    else if (strcmp(tr, "help"))
        printf("help: [cmd|-lt|-T|-all]	Muestra ayuda sobre los comandos"
               "cmd: info sobre el comando cmd");
    else if (strcmp(tr, "chdir"))
        printf("chdir: [dir]	Cambia (o muestra) el directorio actual del shell");
    else if (strcmp(tr, "open"))
        printf("open: fich m1 m2...	Abre el fichero fich"
               "y lo anade a la lista de ficheros abiertos del shell"
               "m1, m2..es el modo de apertura (or bit a bit de los siguientes)"
               "cr: O_CREAT	ap: O_APPEND"
               "ex: O_EXCL 	ro: O_RDONLY"
               "rw: O_RDWR 	wo: O_WRONLY"
               "tr: O_TRUNC");
    else
        printf("faltan comandos");
}

void Cmd_chdir(char *dir) {
    if (dir == NULL)
        MostrarDirActual();
    else if (chdir(dir) == -1)
        perror("Imposible cambiar directorio");
}

void Cmd_open(char *tr[], tList *FicherosAbiertos){
    int df;
    int mode;
    if (tr[0]==NULL) {
        ListarFicherosAbiertos(*FicherosAbiertos);
        return;
    }
    for (int i=1; tr[i]!=NULL; i++)
      if (!strcmp(tr[i],"cr")) mode|=O_CREAT;
      else if (!strcmp(tr[i],"ex")) mode|=O_EXCL;
      else if (!strcmp(tr[i],"ro")) mode|=O_RDONLY;
      else if (!strcmp(tr[i],"wo")) mode|=O_WRONLY;
      else if (!strcmp(tr[i],"rw")) mode|=O_RDWR;
      else if (!strcmp(tr[i],"ap")) mode|=O_APPEND;
      else if (!strcmp(tr[i],"tr")) mode|=O_TRUNC;
      else break;

    if ((df=open(tr[0],mode,0777))==-1)
        perror ("Imposible abrir fichero");
    else{
        if(AñadirAFicherosAbiertos (df, mode, tr[0], FicherosAbiertos)){
            printf ("Anadida entrada a la tabla ficheros abiertos: descriptor-> %d, modo-> %d, nombre-> %s", df, mode, tr[0]);
        }
        else printf("no se pudo añadir a la lista");
    }
}

void Cmd_close(){
    /* Código de Cmd_close */
}

void Cmd_listopen() {
    /* Código de Cmd_listopen */
}

void Cmd_dup() {
    /* Código de Cmd_dup */
}

void Cmd_lseek() {
    /* Código de Cmd_lseek */
}

int Cmd_readstr(int df, int cont) {
    char buffer[cont + 1];              
    ssize_t status = read(df, buffer, cont);

    if (status == -1) {
        perror("read failed");
        return 1;
    }

    buffer[status] = '\0';             

    printf("%s\n", buffer);

    return 0;
}

void Cmd_writestr() {
    /* Código de Cmd_writestr */
}

int Cmd_makefile(char *tr) {
    int status = open(tr, O_CREAT);

    if (status == -1) {
        perror("open failed");
        return 1;
    }

    return 0;
}

int Cmd_makedir(char *tr) {
    int status = mkdir(tr, O_CREAT);

    if (status == -1) {
        perror("mkdir failed");
        return 1;
    }

    return 0;
}

void Cmd_delete() {
    /* Código de Cmd_delete */
}

void Cmd_deltree() {
    /* Código de Cmd_deltree */
}

void Cmd_listfile() {
    /* Código de Cmd_listfile */
}

void Cmd_list() {
    /* Código de Cmd_list */
}