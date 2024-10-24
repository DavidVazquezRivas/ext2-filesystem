/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "directorios.h"
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n");
        return EXIT_FAILURE;
    }

    // Obtener los argumentos
    const char *disco = argv[1];
    const char *ruta_original = argv[2];
    const char *ruta_enlace = argv[3];

    // Montar dispositivo
    bmount(disco);
    
    // Verificar que la ruta original un fichero
    if (ruta_original[strlen(ruta_original) - 1] != '/') {
        if (mi_link(ruta_original, ruta_enlace) > 0) {
            printf("Se ha creado correctamente el enlace");
        }
    } else {
        fprintf(stderr, RED "Error: no es un fichero." RESET);
        return FALLO;
    }

    bumount();
    
    return EXITO;
}