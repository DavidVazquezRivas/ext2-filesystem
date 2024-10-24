/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "ficheros.h"

int main(int argc, char **argv) {

    // Comprobar sintaxis
    if (argc != 4) {
        perror(RED "Sintaxis: ./permitir <nombre_dispositivo> <ninodo> <permisos>" RESET);
        return FALLO;
    }
    
    // Montar disco
    char *nombre = argv[1];
    bmount(nombre);

    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[3]);

    //Cambiamos permisos del inodo
    mi_chmod_f(ninodo, permisos);

    // Desmontar disco
    bumount();
}