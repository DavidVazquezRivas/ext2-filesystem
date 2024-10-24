/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "directorios.h"

int main(int argc, char **argv) {

    char buffer[TAMBUFFER];
    // Inicializar el buffer a ceros
    memset(buffer, 0, TAMBUFFER);

    if (argc != 3 && argc != 4) {
        perror(RED "Sintaxis: ./mi_ls (-l) <nombre_disco> </ruta>" RESET);
        return FALLO;
    }

    // Seleccionar si se ha de mostrar información detallada o no
    int modo = 0;
    if (argc == 4) modo = 1;
    
    // Montar disco
    bmount(argv[1 + modo]);

    // Seleccionar tipo de directorio/fichero
    char tipo = argv[2 + modo][strlen(argv[2 + modo]) - 1] == '/' ? 'd' : 'f';

    // Seleccionar path
    char* path = argv[2 + modo];

    int entradas = mi_dir(path, buffer, tipo, modo);
    if (entradas > 0) {
        fprintf(stdout, "Total: %d\n", entradas);
        if (modo) {
            // Imprimir cabecera
            fprintf(stdout, "Tipo\tPermisos\tmTime\t\t\tTamaño\t\tNombre\n--------------------------------------------------------------------------------------------\n");
        }
        
        fprintf(stdout, "%s\n", buffer);
    }

    // Desmontar disco
    bumount();
}