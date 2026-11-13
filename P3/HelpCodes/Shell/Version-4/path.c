#include "path.h"


static LISTASIMPLE S;    /*y el path */

int Comparar (void *a, void *b)   /*la funcion de comparación para la busqueda*/
{
    return strcmp ((char *)a, (char *) b);
}

void ImprimirDir (void *dir)      /*la funcion que imprime un elemento*/ 
{
     printf ("%s\n",(char *) dir); 
}


/****************************El PATH **************************/
int PathAdd (char * dir)
{
   char *s;
   if (BuscarElemento(S,dir,Comparar)!=-1) /*el elemento ya esta en el path*/
        return 0;
   if ((s=strdup(dir))==NULL)  /*no puede asignarse*/
        return -1;
   return AniadirElemento (S,(void *)s);
}

void PathClear(void)
{
    BorrarLista(S);
}

int PathDel(char *dir)
{
    return BorrarElemento(S,(void *) dir,Comparar);
}

char * PathFirst (void)
{
   return GetPrimerElemento (S);
}

char * PathNext (void)
{
   return GetSiguienteElemento (S);
}

void PathPrint(void)
{
   ImprimirListaCompleta(S,0,ImprimirDir);
}

int PathAddPath (void)
{
#define MAXPATH 2048         /*longitud maxima del PATH*/
    char * p;
    int i=1;
    char aux[MAXPATH]; 
    if ((p=getenv("PATH"))==NULL)
        return 0;
    strncpy(aux,p,MAXPATH-1);
    if ((p=strtok(aux,":"))!=NULL)
        PathAdd(p);
    while ((p=strtok(NULL,":"))!=NULL){
        PathAdd(p);
        i++;
    }
    return i;   /*devolvemos cuantos directorios se ha aniadido*/
}

char * Ejecutable (char * s)  /*devuelve la trayectoria completa a un ejecutable*/
{                               /*si esta en el PATH*/
#define   MAXNAME  1024                             
	static char path[MAXNAME];
	struct stat st;
    char *p;
    
	if (s==NULL || (p=(char *)PathFirst())==NULL)
		return s;
	if (s[0]=='/' || !strncmp (s,"./",2) || !strncmp (s,"../",3))
        return s;        /*is an absolute pathname*/
        
	snprintf (path,MAXNAME-1,"%s/%s",p,s);
	if (lstat(path,&st)!=-1)
		return path;
	while ((p=(char *) PathNext())!=NULL){
	    snprintf (path,MAXNAME-1,"%s/%s",p,s);
	    if (lstat(path,&st)!=-1)
		   return path;
	}
	return s;
}
