#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


#define MAXENTRADA 2048
#define MAXNOMBRE 1024


struct COMANDO{
  char * nombre;
  void (*funcion)(char**);
};

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
void Cmd_fin (char * arg[])  /*todos los cmd_ comparten prototipo*/
{                            /*reciben los mismos parametros aunque no los usen*/
    exit(0);
}

void Cmd_autores(char *arg[])
{
  printf ("los autores del shell....\n");
}

void Cmd_exec (char *arg[])
{
  if (execvp(arg[0],arg)==-1)
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

void Cmd_chdir (char * arg[])
{
   if (arg[0]==NULL)
      MostrarDirActual();
   else if (chdir(arg[0])==-1)
      perror("Imposible cambiar directorio");
}

void Cmd_pwd(char * arg[])
{
    MostrarDirActual();
}

void Cmd_pid (char * arg[])
{
    if (arg[0]==NULL)
        printf ("El pid del proceso es %d\n",(int) getpid());
    else
        if (!strcmp (arg[0],"-p"))
            printf ("El pid del proceso padre es %d\n",(int) getppid());
}


/**************************SHELL**************************/

static struct COMANDO C[]={ /*no declaro dimension, que la coge de la inicializacion*/
   {"fin",Cmd_fin},
   {"exit",Cmd_fin},
   {"quit",Cmd_fin},
   {"pid",Cmd_pid},
   {"pwd", Cmd_pwd},
   {"chdir",Cmd_chdir},
   {"autores",Cmd_autores},
   {"exec",Cmd_exec},
   {"pplano",Cmd_pplano},
   {"splano",Cmd_splano},
   {NULL,NULL},             /*NULL marca el final del array*/
  };

void DecidirComando (char *tr[])
{
  int i;

  if (tr[0]==NULL) return; /*por si luego quito lode TrocearCadena ==0*/
  for (i=0; C[i].nombre!=NULL; i++) /*recorro el array hasta el NULL*/
    if (!strcmp(C[i].nombre,tr[0])){
        (*C[i].funcion)(tr+1);
	    return; /*si es algun cmd_ ya no miro mas*/
    }
  Proceso(tr,0); /*si no es un comando, es un  ejecutable externo*/
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
