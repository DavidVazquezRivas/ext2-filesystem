/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "directorios.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Sintaxis:: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return FALLO;
    }

    // Obtener argumentos
    char *nombre = argv[1];
    char *ruta_fichero = argv[2];
    char *texto = argv[3]; 
    int offset = atoi(argv[4]);
    int nbytes = strlen(texto);

    // Montar disco
    bmount(nombre);

    printf("Longitud texto: %d\n", nbytes);
    
    // Escribir en el fichero
    int bytes_escritos = mi_write(ruta_fichero, texto, offset, nbytes);

    if (bytes_escritos < 0) {
        fprintf(stderr, RED "Error al escribir en el fichero\n" RESET);
        bumount();
        return FALLO;
    }

    printf("Bytes escritos: %d\n", bytes_escritos);

    // Desmontar disco
    bumount();
}
