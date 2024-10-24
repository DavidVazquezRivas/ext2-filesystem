/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "directorios.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, RED "Sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n" RESET);
        return FALLO;
    }

    // Montar el dispositivo
    bmount(argv[1]);

    // Obtener los argumentos
    const char *camino = argv[3];
//    printf("Usuario: ");
//    imprimir_cadena_con_no_visibles(camino);
    unsigned char permisos = (unsigned char)atoi(argv[2]);

    // Comprobar que los permisos son válidos
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED "Permiso inválido: debe ser un número entre 0 y 7.\n" RESET);
        fprintf(stderr, RED "Sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n" RESET);
        
        return FALLO;
    }
    
    // Cambiar los permisos
    if (mi_chmod(camino, permisos) == -1) {
        bumount(); 
        return FALLO;
    }

    // Desmontar el dispositivo
    bumount(); 

    printf("Permisos cambiados correctamente.\n");

    return EXITO;
}