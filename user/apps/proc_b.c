#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "syscalls.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char *argv[]) {
    char *filename = "/fork_fs.txt";
    char buffer[128];
    char *mensaje = "Persistencia de estado confirmada mediante descriptor heredado.";

    int fd = open(filename, 3);
    if (fd < 0) {
        printf("Fallo de asignacion en Filesystem.\n");
        return -1;
    }

    printf("Archivo inicializado. FD: [%d]\n", fd);

    int pid = sys_fork();
    
    if (pid < 0) {
        printf("Fallo en sys_fork.\n");
        return -1;
    }

    if (pid == 0) {
        printf("Hijo [PID %d] inyectando bloque de datos...\n", getpid());
        
        if (write(fd, mensaje, strlen((const uint8_t*)mensaje)) < 0) {
            printf("Hijo [PID %d] fallo en operacion de escritura.\n", getpid());
        }
        
        if (close(fd) < 0) {
            printf("Hijo [PID %d] fallo en cierre de descriptor.\n", getpid());
        }
        
        exit(0);
    } else {
        wait(pid);
        printf("Padre [PID %d] reanudando ejecucion. Extrayendo estado mutado por hijo...\n", getpid());
        
        if (lseek(fd, 0, 0) < 0) {
            printf("Padre [PID %d] fallo en reubicacion de offset logico.\n", getpid());
        }

        int bytes_leidos = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_leidos > 0) {
            buffer[bytes_leidos] = '\0';
            printf("Padre [PID %d] extrajo: %s\n", getpid(), buffer);
        } else {
            printf("Padre [PID %d] fallo en operacion de lectura.\n", getpid());
        }
        
        if (close(fd) < 0) {
            printf("Padre [PID %d] fallo en cierre de descriptor.\n", getpid());
        }

        unlink(filename);
        printf("Sincronizacion y destruccion de recursos completada.\n");
    }

    return 0;
}