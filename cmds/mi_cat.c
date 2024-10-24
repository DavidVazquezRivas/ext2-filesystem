/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "directorios.h"

#define TAM_BUFFER BLOCKSIZE


#define tambuffer 4096 // Tamaño del buffer de lectura

int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_escribir <disco> </ruta_fichero>\n ");
        return FALLO;
    }



    // Obtener argumentos
    char *nombre = argv[1];
    char *ruta_fichero = argv[2];

    if (ruta_fichero[strlen(ruta_fichero) - 1] == '/') {
        fprintf(stderr, "No se puede imprimir un directorio\n");
        return FALLO;
    }


    // Montar disco
    bmount(nombre); 

    char buffer_texto[tambuffer];

    memset(buffer_texto, 0, tambuffer);
    
    // Variables para la lectura
    int offset = 0;
    int leidos = 0;
    
    leidos = mi_read(ruta_fichero, buffer_texto, offset, tambuffer);
    
    while (leidos > 0) {
        // Escribir en la salida estándar
        if (write(1, buffer_texto, leidos) == FALLO) {
            fprintf(stderr, "Error al escribir en la salida estandar\n");
            return FALLO;
        }

        // Limpiar el buffer de lectura antes de la próxima lectura
        memset(buffer_texto, 0, sizeof(buffer_texto));

        // Actualizar el offset para la siguiente lectura
        offset += leidos;

        // Leer del inodo
        leidos = mi_read(ruta_fichero, buffer_texto, offset, tambuffer);
    }

    fprintf(stderr, "\n\ntotal_leidos %d\n", offset);
   



    // Desmontar disco
    bumount();
}

