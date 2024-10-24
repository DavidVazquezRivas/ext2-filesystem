/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "ficheros.h"

#define TAM_BUFFER BLOCKSIZE


#define tambuffer 1500 // Tamaño del buffer de lectura

int main(int argc, char *argv[]) {
    // Comprobar número correcto de argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <nombre_dispositivo> <ninodo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Obtener el número de inodo y el nombre del dispositivo
    unsigned int ninodo = atoi(argv[2]);
    char *nombre_dispositivo = argv[1];

    // Inicializar el sistema de ficheros
    bmount(nombre_dispositivo);

    // Obtener la información del inodo
    struct STAT stat_inodo;
    mi_stat_f(ninodo, &stat_inodo);

    // Variables para la lectura
    int offset = 0;
    int leidos = 0;

    // Leer el fichero bloque a bloque y mostrarlo por pantalla
    char buffer_texto[tambuffer];
    memset(buffer_texto, 0, tambuffer);
    leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    while (leidos > 0) {

        // Limpiar el buffer de lectura antes de la próxima lectura
        memset(buffer_texto, 0, tambuffer);

        // Actualizar el offset para la siguiente lectura
        offset += leidos;

        // Leer del inodo
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    }

    // Mostrar el número de bytes leídos y la información del inodo
    fprintf(stderr, "\n\ntotal_leidos %d\n", offset);
    fprintf(stderr, "tamEnBytesLog %d\n", stat_inodo.tamEnBytesLog);

    // Desmontar el dispositivo
    bumount() ;
       

    return EXITO;
}