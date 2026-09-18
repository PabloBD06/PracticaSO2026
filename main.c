// Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
// Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>


#define MAXENTRADA  2048
#define MAXNOMBRE 1024

void MostrarDirActual()
{
   char dir[MAXNOMBRE];
   
    if (getcwd(dir,MAXNOMBRE)==NULL)
	perror("Imposible obtener directorio");
    else
       printf ("%s\n",dir);
}

int ComprobarSegundoPlano (char *tr[])
{
    int i;
    for (i=0; tr[i]!=NULL;i++)
        if (!strcmp(tr[i],"&")){ /*& indica segundo plano*/ 
            tr[i]=NULL;         /*es el ultimo argumento*/
            return i;       /*si solo hay un & no se ejecuta nada en pplano*/
            }
    return 0;
}


void Proceso (char *tr[], int splano)
{
   pid_t pid;
   void Cmd_exec (char **);
   int background=splano || ComprobarSegundoPlano(tr);
   if ((pid=fork())==-1){
        perror ("Imposible crear proceso");
        return;
        }
  if (pid==0){  /*proceso hijo*/
    Cmd_exec (tr);
    exit(255); /*por si falla exec*/
    }
  if (!background) 
    waitpid(pid,NULL,0);
}

/*********************************************/
/*************COMANDOS DEL SHELL************************/
  
void Cmd_autores(char *tr) {
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
    else if (strcmp(tr, "exit") || strcmp(tr, "exit"))
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
void Cmd_exec (char *tr[])
{
  if (execvp(tr[0],tr)==-1)
	perror ("Imposible ejecutar");
}

void Cmd_splano (char *arg[])
{
  Proceso (arg,1);
}
void Cmd_pplano (char *arg[])
{
  Proceso(arg,0);
}

void Cmd_chdir (char * dir)
{
   if (dir==NULL)
      MostrarDirActual();
   else if (chdir(dir)==-1)
      perror("Imposible cambiar directorio");
}

void Cmd_pwd()
{
    MostrarDirActual();
}

void Cmd_pid (char * arg)
{
    if (arg==NULL)
        printf ("El pid del proceso es %d\n",(int) getpid());
    else
        if (!strcmp (arg,"-p"))
            printf ("El pid del proceso padre es %d\n",(int) getppid());
}

/**************************SHELL**************************/
void DecidirComando(char *tr[])
{
  if (tr[0]==NULL)  /*por si cambiamos lo de TroearCadena==0*/
    return;         /*no hace falta que ya comprobamos que TrocearCadena no devielve 0*/
  if (!strcmp(tr[0],"quit") || !strcmp(tr[0],"exit")) exit(0);
  else if (!strcmp(tr[0],"date")) Cmd_autores(tr[1]);
  else if (!strcmp(tr[0],"pid")) Cmd_pid (tr[1]);
  else if (!strcmp(tr[0],"authors")) Cmd_autores(tr[1]);
  else if (!strcmp(tr[0],"sysinfo")) Cmd_sysinfo();
  else if (!strcmp(tr[0],"help")) ;
  else if (!strcmp(tr[0],"chdir")) ;
  else if (!strcmp(tr[0],"open")) ;
  else if (!strcmp(tr[0],"close")) ;
  else if (!strcmp(tr[0],"listopen")) ;
  else if (!strcmp(tr[0],"dup")) ;
  else if (!strcmp(tr[0],"lseek")) ;
  else if (!strcmp(tr[0],"readstr")) ;
  else if (!strcmp(tr[0],"writestr")) ;
  else if (!strcmp(tr[0],"makefile")) ;
  else if (!strcmp(tr[0],"makedir")) ;
  else if (!strcmp(tr[0],"delete")) ;
  else if (!strcmp(tr[0],"deletetree")) ;
  else if (!strcmp(tr[0],"listfile")) ;
  else if (!strcmp(tr[0],"list")) ;
  else Cmd_pplano(tr);
}

int TrocearCadena(char * cadena, char * trozos[])
{
  int i=1;
  if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
      return 0;
  while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
     i++;
  return i;
}	
void ProcesarEntrada(char * entrada)
{
   char *tr[MAXENTRADA/2];
   if (TrocearCadena(entrada,tr)==0) /*no hay nada*/
	return;
   DecidirComando(tr);
}

int main(int argc, char *argv[], char *ent[])
{
   char entrada[MAXENTRADA];

   while (1){
      printf ("-> ");
      fgets(entrada,MAXENTRADA,stdin);
      ProcesarEntrada(entrada);
   }
}