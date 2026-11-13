#ifndef EJEMPLO_H
#define EJEMPLO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "path.h"

#define MAXNOMBRE 1024

void Cmd_fin (char * arg[]);
void Cmd_autores(char *arg[]);
void Cmd_exec (char *arg[]);
void Cmd_splano (char *arg[]);
void Cmd_pplano (char *arg[]);
void Cmd_chdir (char * arg[]);
void Cmd_pwd(char * arg[]);
void Cmd_pid (char * arg[]);
void Cmd_path (char * arg[]);
void Cmd_importpath (char *arg[]);
void Cmd_where (char *args[]);



#endif
