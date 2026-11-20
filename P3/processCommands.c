#include "processCommands.h"
#include <pwd.h>
#include <stdio.h>
#include <sys/resource.h>:
# include <errno.h>
void Cmd_getuid(){
    uid_t uid = getuid();
    struct passwd *usuario = getpwuid(uid);
    printf("Credencial real: %d (%s)\n", uid, usuario ? usuario->pw_name : "Desconocido");
    uid_t euid = geteuid();
    usuario = getpwuid(euid);
    printf("Credencial efectiva: %d (%s)\n", euid, usuario ? usuario->pw_name : "Desconocido");
}
void Cmd_setuid(char *tr[]){
//Si no hay argumentos se imprimen las credenciales
    if (tr[0] == NULL) {
        Cmd_getuid();
        return;
    }
    if (strcmp(tr[0], "-l") == 0) {
        if (tr[1] == NULL) {
            Cmd_getuid();
            return;
        }
        char *login = tr[1];
        struct passwd *usuario = getpwnam(login);
        if(usuario == NULL) {
            perror("getpwnam");
            return;
        }
        if(seteuid(usuario->pw_uid) == -1) {
            perror("seteuid");
            return;
        }
        return;
    } else{ uid_t uid = atoi(tr[0]);
        if(seteuid(uid) == -1) {
            perror("seteuid");
            return;
        }
        return;
    }
}
void Cmd_prio(char *tr[]){
    errno = 0;
    pid_t pid;
    if(tr[0] == NULL) {
        pid = getpid();
    } else {
        pid = atoi(tr[0]);
    }
    if(tr[1] == NULL){
        int prio = getpriority(PRIO_PROCESS, pid);
        if (prio == -1 && errno != 0) {
            perror("getpriority");
            return;
        }
        printf("Prioridad del proceso %d es %d\n", pid, prio);
    } else {
        int new_prio = atoi(tr[1]);
        if (setpriority(PRIO_PROCESS, pid, new_prio) == -1) {
            perror("setpriority");
            return;
        }
        printf("Prioridad del proceso %d cambiada a: %d\n", pid, new_prio);
    }
}
void Cmd_path(char *tr[]){
    
}
void Cmd_showenv(char *tr[]){}
void Cmd_showvar(char *tr[]){}
void Cmd_changevar(char *tr[]){}
void Cmd_fork(char *tr[]){}
void Cmd_ejec(char *tr[]){}
void Cmd_ejecpri(char *tr[]){}
void Cmd_pplano(char *tr[]){}
void Cmd_pplanopri(char *tr[]){}
void Cmd_splano(char *tr[]){}
void Cmd_splanopri(char *tr[]){}
void Cmd_jobs(char *tr[]){}
void Cmd_deljobs(char *tr[]){}