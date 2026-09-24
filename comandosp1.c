#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>

#include "comandosp1.h"

#define MAXNOMBREDIR 1024 //tamaño maximo de nombre de directorio

/**
 * Función auxiliar usada en Cmd_chdir
 */
void MostrarDirActual()
{
   char dir[MAXNOMBREDIR];
   
    if (getcwd(dir,MAXNOMBREDIR)==NULL)
	perror("Imposible obtener directorio");
    else
       printf ("%s\n",dir);
}

void Cmd_date(char *tr){
    time_t now = time(NULL);
    struct tm *t  = localtime(&now);

    if (!tr) {
        char date[21];
        strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", t);
        printf("%s\n", date);
    }
    else if (!strcmp(tr, "-d")) {
        char date[11];
        strftime(date, sizeof(date), "%Y-%m-%d", t);
        printf("%s\n", date);
    }
    else if (!strcmp(tr, "-t")) {
        char hour[9];
        strftime(hour, sizeof(hour), "%H:%M:%S", t);
        printf("%s\n", hour);
    }    
}


void Cmd_pid(char *arg){
    if (arg==NULL)
        printf ("El pid del proceso es %d\n",(int) getpid());
    else
        if (!strcmp (arg,"-p"))
            printf ("El pid del proceso padre es %d\n",(int) getppid());
}


void Cmd_autores(char *tr){
    if (!tr)
        printf("Pablo Bea Dopazo(pablo.bea.dopazo)\nSzymon Artur Zieba Glaz(szymon.zieba)\n");
    else if (strcmp(tr, "-n"))
        printf("Pablo Bea Dopazo\nSzymon Artur Zieba Glaz\n");
    else if (strcmp(tr, "-l"))
        printf("pablo.bea.dopazo\nszymon.zieba\n");
}


void Cmd_sysinfo(){
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


void Cmd_help(char *tr){
    if(!tr){
        printf("Cmds practica 0: prompt numserie limits systemd command historic\n" 
                "Cmds practica 1: exit bye date pid authors sysinfo help chdir open close listopen dup lseek readstr writestr makefile makedir delete deltree listfile list\n"
                "Cmds practica 2: allocate deallocate i-o read write memory recurse memfill memdump\n"
                "Cmds practica 3: getuid setuid prio path showenv showvar changevar changevarWrong fork ejec ejecpri pplano pplanopri splano splanopri jobs deljobs\n");
    }
    else if (strcmp(tr, "exit") || strcmp(tr, "bye"))
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


void Cmd_chdir(char *dir){
    if (dir==NULL)
      MostrarDirActual();
   else if (chdir(dir)==-1)
      perror("Imposible cambiar directorio");
}


void Cmd_open (char * tr[]){
    /*
    if (tr[0]==NULL) { 
        ListarFicherosAbiertos();
        return;
    }
    for (i=1; tr[i]!=NULL; i++)
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
        AnadirAFicherosAbiertos (descriptor...modo...nombre....);
        printf ("Anadida entrada a la tabla ficheros abiertos..................",......); 
    */
}


void Cmd_close()
{
    /* Código de Cmd_close */
}


void Cmd_listopen()
{
    /* Código de Cmd_listopen */
}


void Cmd_dup()
{
    /* Código de Cmd_dup */
}


void Cmd_lseek()
{
    /* Código de Cmd_lseek */
}


void Cmd_readstr()
{
    /* Código de Cmd_readstr */
}


void Cmd_writestr()
{
    /* Código de Cmd_writestr */
}


void Cmd_makefile()
{
    /* Código de Cmd_makefile */
}


void Cmd_makedir()
{
    /* Código de Cmd_makedir */
}


void Cmd_delete()
{
    /* Código de Cmd_delete */
}


void Cmd_deltree()
{
    /* Código de Cmd_deltree */
}


void Cmd_listfile()
{
    /* Código de Cmd_listfile */
}


void Cmd_list()
{
    /* Código de Cmd_list */
}