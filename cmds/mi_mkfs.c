/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "directorios.h"

int main(int argc, char **argv) {

    unsigned char buffer[BLOCKSIZE];
    // Inicializar el buffer a ceros
    memset(buffer, 0, BLOCKSIZE);

    if (argc != 3) {
        perror(RED "Sintaxis: ./mi_mkfs <nombre_disco> <número de bloques>" RESET);
        return FALLO;
    }
    
    // Montar disco
    char *nombre = argv[1];
    bmount(nombre);

    // Inicializar disco
    unsigned int nBloques = atoi(argv[2]);
    for(int i = 0; i < nBloques; i++) {
        bwrite(i, buffer);
    }

    // Inicializar metadatos
    initSB(nBloques, nBloques / 4);
    initMB();
    initAI();

    // Crear directorio raiz
    reservar_inodo('d',7);

    // Desmontar disco
    bumount();
}