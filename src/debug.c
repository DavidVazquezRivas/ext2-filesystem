/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

// Aquí guardamos funciones utiles para nosotros desbugear cosas, no se entrega.

#include <stdio.h>

void printblock(char *block, int length) {
    for (int i = 0; i < length; i++) {
        fprintf(stderr, "%d ", (int)*(block + i));
    }
    fprintf(stderr, "\n");
    fflush(stderr);
}