#ifndef COMANDOSP1_H
#define COMANDOSP1_H

void Cmd_date(char *tr);
void Cmd_pid(char *arg);
void Cmd_autores(char *tr);
void Cmd_sysinfo();
void Cmd_help(char *tr);
void Cmd_chdir(char *dir);
void Cmd_open(char *tr[]);
void Cmd_close();
void Cmd_listopen();
void Cmd_dup();
void Cmd_lseek();
void Cmd_readstr();
void Cmd_writestr();
int Cmd_makefile(char *tr);
int Cmd_makedir(char *tr);
void Cmd_delete();
void Cmd_deltree();
void Cmd_listfile();
void Cmd_list();

#endif