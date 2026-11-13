#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


#define MAXENTRADA 2048
#define MAXNOMBRE 1024

#define NONE 0
#define FIN 1
#define PID 2
#define PWD 3
#define CHDIR 4
#define AUTORES 5
#define EXEC 6
#define PPLANO 7
#define SPLANO 8


void MostrarDirActual()
{
  char dir[MAXNOMBRE];
  if (getcwd(dir,MAXNOMBRE)==NULL)
	perror("imposible obtener directorio actual");
  else
	printf ("%s\n",dir);
}


int ComprobarSegundoPlano (char *tr[])
{
    int i;
    for (i=0; tr[i]!=NULL;i++)
        if (!strcmp(tr[i],"&")){ /*& indica segund plano*/ 
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
  
  
void Cmd_autores()
{
  printf ("Los autores del shell....\n");
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


void Cmd_pid(char * arg)
{
  if (arg==NULL)
       printf ("Pid de shell: %d\n",getpid());
  else if (!strcmp ("-p",arg))
      printf ("Pid del padre del shell: %d\n",getppid());
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

int  CodigoEntrada (char *tr)
{
  if (tr==NULL)  return NONE; /*ademas comprobamos Trocearcadena==0*/
  if (!strcmp(tr,"fin")) return FIN;
  if (!strcmp(tr,"quit")) return FIN;
  if (!strcmp(tr,"exit")) return FIN;
  if (!strcmp(tr,"pid")) return PID;
  if (!strcmp(tr,"pwd")) return PWD;
  if (!strcmp(tr,"chdir")) return CHDIR;
  if (!strcmp(tr,"autores")) return AUTORES;
  if (!strcmp(tr,"exec")) return EXEC;
  if (!strcmp(tr,"pplano")) return PPLANO;
  if (!strcmp(tr,"splano")) return SPLANO;
  return NONE;
}

void ProcesarEntrada (char *tr[])
{
  switch (CodigoEntrada(tr[0])){
     case FIN: exit (0); break;
     case PID:  Cmd_pid(tr[1]); break;
     case PWD:  Cmd_pwd();break;
     case CHDIR: Cmd_chdir(tr[1]); break;
     case AUTORES: Cmd_autores(); break;
     case EXEC: Cmd_exec(tr+1);break;
     case SPLANO: Cmd_splano(tr+1);break;
     case PPLANO: Cmd_pplano (tr+1);break;
     default: Cmd_pplano (tr);
  }
}

int main (int argc, char *argv[])
{
  char entrada[MAXENTRADA];
  char *tr[MAXENTRADA/2];

while (1) {
   printf ("-> ");
   fgets (entrada,MAXENTRADA,stdin);
   if (TrocearCadena(entrada,tr)==0)
	continue;
   ProcesarEntrada(tr);
  }
}
