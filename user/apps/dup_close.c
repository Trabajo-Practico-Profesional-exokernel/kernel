#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdio.h"

#define STDOUT 1

void main() {
    int fds[2];
    char buffer[128];
    
    printf("=== Prueba de redireccion con CLOSE y DUP en Pipes ===\n");

    int saved_stdout = dup(STDOUT);
    if (saved_stdout < 0) {
        printf("Fallo clonacion de STDOUT original.\n");
        return;
    }
    
    if (pipe(fds) < 0) {
        printf("Fallo en inicializacion de pipe.\n");
        return;
    }

    printf("Pipe abierto en lectura FD [%d] y escritura FD [%d]. Ejecutando close(1)...\n", fds[0], fds[1]);
    
    close(STDOUT);
    
    int new_fd = dup(fds[1]);
    
    if (new_fd != STDOUT) {
        close(new_fd);
        dup(saved_stdout); 
        close(saved_stdout);
        printf("Fallo arquitectonico: dup() asigno el FD [%d] en lugar del FD [1] liberado.\n", new_fd);
        return;
    }

    printf("Redireccion exitosa. Este texto reside en el buffer de la tuberia.\n");
    char *msg = "Texto escrito en tuberia mediante write(1, ...)\n";
    write(STDOUT, msg, strlen((const uint8_t*)msg));

    close(STDOUT);
    dup(saved_stdout);
    close(saved_stdout);

    printf("Consola restaurada. Extrayendo contenido de la tuberia...\n");
    printf("----------------------------------------\n");
    
    int bytes = read(fds[0], buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("%s", buffer);
    } else {
        printf("[Tuberia vacia o fallo de lectura]\n");
    }

    close(fds[0]);
    close(fds[1]);

    printf("----------------------------------------\n");
}
