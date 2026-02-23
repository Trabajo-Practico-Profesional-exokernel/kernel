#include "syscalls.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char *argv[]) {
    int fds[2];
    char buffer[128];
    char *mensaje = "Sincronizacion IPC exitosa mediante fork.";

    if (pipe(fds) < 0) {
        printf("Fallo en inicializacion de pipe.\n");
        return -1;
    }
    printf("fd[0] : [%d] - fd[1] : [%d] \n", fds[0], fds[1]);
    int pid = sys_fork();
    printf("pid obtenido [%d]\n", pid);
    if (pid < 0) {
        printf("Fallo en sys_fork.\n");
        return -1;
    }

    if (pid == 0) {
        printf("hijo cerrando fd 1\n");
        if (close(fds[1])<0){
            printf("hijo error en close 1\n");
        }
        printf("proceso hijo leyendo...\n");
        int bytes_leidos = read(fds[0], buffer, sizeof(buffer) - 1);
        if (bytes_leidos > 0) {
            buffer[bytes_leidos] = '\0';
            printf("Proceso Hijo [PID %d] recibio: %s\n", getpid(), buffer);
        } else {
            printf("Proceso Hijo [PID %d] fallo al leer.\n", getpid());
        }
        
        close(fds[0]);
        exit(0);
    } else {
        if (close(fds[0])<0){
            printf("error en close 0\n");
        }
        
        printf("Proceso Padre [PID %d] transmitiendo estado...\n", getpid());
        if (write(fds[1], mensaje, strlen((const uint8_t*)mensaje))<0){
            printf("ERROR TRANSMITIENDO ESTADO\n");
        };
        
        if (close(fds[1])<0){
            printf("error en close 1\n");
        }
        
        wait(pid);
        printf("Proceso Padre [PID %d] finalizo sincronizacion.\n", getpid());
    }

    return 0;
}
