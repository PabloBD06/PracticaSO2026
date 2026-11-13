#ifndef PATH_H
#define PATH_H

#include "listasimple.h"
#include <string.h>
#include <sys/stat.h>

/*el PATH lo guardo como una lista simple de dirs*/





int PathAdd (char * dir);  /*añade un dir al path*/
void PathClear(void);      /*vacia el path*/
int PathDel(char *dir);    /*elimina un dir del path*/
char * PathFirst (void);
char * PathNext (void);
void PathPrint(void);       /*imprime el path*/
int PathAddPath (void);     /*importa los directorios de la variable de entorno PATH al path*/
char * Ejecutable (char * ejec); /*busca un ejecutable en el path y devuelve la trayectoria completa hasta el*/



#endif

