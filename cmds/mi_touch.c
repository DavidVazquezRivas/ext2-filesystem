/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "directorios.h"

int main(int argc, char **argv){
    
    unsigned char buffer[BLOCKSIZE];
    // Inicializar el buffer a ceros
    memset(buffer, 0, BLOCKSIZE);

    if (argc != 4) {
        fprintf(stderr, RED "Sintaxis: ./mi_mkdir <nombre_disco> <permisos> </ruta>" RESET);
        return FALLO;
    }

    // Montar disco
    char *nombre = argv[1];
    bmount(nombre);

    // Verificar permisos, entero entre 0 y 7.
    int permisos = atoi(argv[2]);
    if (permisos > 7) {
        fprintf(stderr, RED "Error: Modo inválido: <<%d>>" RESET,permisos);
        return FALLO;
    }

    char *ruta = argv[3];
//    printf("Usuario: ");
//    imprimir_cadena_con_no_visibles(ruta);

    // Verificar que es un fichero
    if (ruta[strlen(ruta)-1] != '/') {
        mi_creat(ruta, permisos);
    } else {
        fprintf(stderr, RED "Error: no es un fichero." RESET);
        return FALLO;
    }

    // Desmontar disco
    bumount();
}