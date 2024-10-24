/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "ficheros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
void imprimirstat(int ninodo);


int main(int argc, char *argv[]) {
    int offsets[]={9000,209000,30725000, 409605000, 480000000};

    if (argc != 4) {
        perror(RED "No se han introducido los parametros correctos. Uso: ./escribir <nombre_disco> <\"$(cat fichero)\"> <diferentes_inodos>\nOffsets: 9000, 209000, 30725000, 409605000, 480000000 \nSi diferentes_inodos=0 se reserva un solo inodo para todos los offsets" RESET);
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    char *texto = argv[2]; 
    int diferentes_inodos = atoi(argv[3]);

    printf("Longitud del texto %d\n", (int) strlen(texto));

    bmount(nombre_dispositivo);

    // Escribir el texto en el inodo
    int offset = 0;
    int size = sizeof(offsets) / sizeof(offsets[0]);

    if (diferentes_inodos == 0) {    
        int ninodo = reservar_inodo('f', 6);

        for (int i = 0; i < size ; i++) {
            offset = offsets[i];

            printf("\nNúmero de inodo reservado: %d\n", ninodo);
            printf("Offset : %d\n", offset);

            int bytes_escritos = mi_write_f(ninodo, texto, offset, strlen(texto));

            printf("Bytes escritos: %d\n", bytes_escritos);
            imprimirstat(ninodo);

            if (bytes_escritos == FALLO) {
                perror("Error al escribir en el inodo");
                return FALLO;
            }
        } 
    
    } else {
        
        // Reservar un inodo diferente para cada offset;
        for (int i = 0; i < size; i++) {
            offset = offsets[i]; 

            int inodo_actual = reservar_inodo('f', 6);
            printf("\nNúmero de inodo reservado: %d\n", inodo_actual);
            printf("Offset: %d\n", offset);
            if (inodo_actual == FALLO) {
                perror("Error al reservar el inodo");
                return FALLO;
            }

            int bytes_escritos = mi_write_f (inodo_actual, texto, offset, strlen(texto));

            printf("Bytes escritos: %d\n", bytes_escritos);
            if (bytes_escritos == FALLO) {
                perror("Error al escribir en el inodo");
                return FALLO;
            }

            imprimirstat(inodo_actual);
            
        }
    }   

    // Realizar desmontaje del dispositivo
    bumount();

    return EXITO;
}

void imprimirstat(int ninodo){
    struct STAT stat;
    if (mi_stat_f(ninodo, &stat) == FALLO) {
        perror("Error al obtener el estado del inodo");
        exit(FALLO);
    }
    printf("stat.tamEnBytesLog: %d bytes\n", stat.tamEnBytesLog);
    printf("stat.numBloquesOcupados:  %d\n", stat.numBloquesOcupados); 
}