/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "../include/bloques.h"

#include "semaforo_mutex_posix.h"
static sem_t *mutex;
static unsigned int inside_sc = 0;


static int descriptor = 0;

/******************************************************************************
 * NIVEL 1
 *****************************************************************************/

int bmount(const char *camino) {

    if (descriptor > 0) {
       close(descriptor);
   }

    if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
       mutex = initSem(); 
       if (mutex == SEM_FAILED) {
           return -1;
       }
   }

    // Abrir fichero
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    if (descriptor == FALLO) perror("Error al abrir el archivo");
    
    return descriptor;  
}

int bumount() {
    // Cerrar fichero
    descriptor = close(descriptor);
    deleteSem(); 

    if (descriptor == FALLO) {
        perror("Error al cerrar el archivo");
        return FALLO;
    }

    
    return EXITO;
}

int bwrite(unsigned int nbloque, const void *buf) {
    // Mover puntero al bloque
    lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);

    // Escribir el bloque
    int writtenBytes = write(descriptor, buf, BLOCKSIZE);
    if (writtenBytes == FALLO) perror("Error al escribir el bloque");

    return writtenBytes;
}

int bread(unsigned int nbloque, void *buf) {
    // Mover puntero al bloque
    lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);

    // Leer bloque
    int readenBytes = read(descriptor, buf, BLOCKSIZE);
    if (readenBytes == FALLO) perror("Error al leer el bloque");

    return readenBytes;
}

void mi_waitSem() {
   if (!inside_sc) { // inside_sc==0, no se ha hecho ya un wait
       waitSem(mutex);
   }
   inside_sc++;
}


void mi_signalSem() {
   inside_sc--;
   if (!inside_sc) {
       signalSem(mutex);
   }
}
