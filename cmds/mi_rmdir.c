/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "directorios.h"

int main(int argc, char **argv){

    if (argc != 3) {
        fprintf(stderr, RED "Sintaxis: ./mi_rm <nombre_disco> </ruta>\n" RESET);
        return FALLO;
    }

    // Montar disco
    char *nombre = argv[1];
    bmount(nombre);

    char *ruta = argv[2];

    // Verificar que es un directorio
    if (ruta[strlen(ruta)-1] == '/') {
        mi_unlink(ruta);
    } else {
        fprintf(stderr, RED "Error: no es un directorio." RESET);
        return FALLO;
    }

    // Desmontar disco
    bumount();
}