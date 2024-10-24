/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "../include/ficheros.h"

////////////////////////////////// NIVEL 5 ////////////////////////////////////

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
    int written = 0;

    // Leer inodo
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    
    // Verificar permisos
    if ((inodo.permisos & 2) != 2) {
       fprintf(stderr, RED "No hay permisos de escritura\n" RESET);
       return FALLO;
    }

    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    
    // Escribir primer bloque
    char buf_bloque[BLOCKSIZE];
    mi_waitSem();
    int bf = traducir_bloque_inodo(ninodo, &inodo, primerBL, 1);
    mi_signalSem();
    //leer_inodo(ninodo, &inodo);
    bread(bf, buf_bloque);
    if (primerBL == ultimoBL) { // el contenido a escribir cabe en un bloque
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        written += nbytes;
    } else { // hay maś bloques
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        written += BLOCKSIZE - desp1;
    }
    bwrite(bf, buf_bloque);

    // Escribir bloques intermedios
    for (int i = 1; primerBL + i < ultimoBL; i++) {
        mi_waitSem();
        bf = traducir_bloque_inodo(ninodo, &inodo, primerBL + i, 1);
        mi_signalSem();
        //leer_inodo(ninodo, &inodo);
        memcpy(buf_bloque, buf_original + (BLOCKSIZE - desp1) + (i - 1) * BLOCKSIZE, BLOCKSIZE);
        bwrite(bf, buf_bloque);
        written += BLOCKSIZE;
    }

    // Escribir bloque final
    if (primerBL != ultimoBL) {
        int desp2 = (offset + nbytes - 1) % BLOCKSIZE; // calcular desplazamiento
        mi_waitSem();
        bf = traducir_bloque_inodo(ninodo, &inodo, ultimoBL, 1);
        mi_signalSem();
        //leer_inodo(ninodo, &inodo);
        bread(bf, buf_bloque);
        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);
        bwrite(bf, buf_bloque);
        written += desp2 + 1;
    }

    // Actualizar metadatos del inodo
    mi_waitSem();
    leer_inodo(ninodo, &inodo); 
    if (offset + nbytes > inodo.tamEnBytesLog) {
        inodo.tamEnBytesLog = offset + nbytes;
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL);
    escribir_inodo(ninodo, &inodo); 
    mi_signalSem();

    return written;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
    int read = 0;

    // Leer inodo
    struct inodo inodo;
    // Actualizar metadatos del inodo
    mi_waitSem();
    leer_inodo(ninodo, &inodo);
    inodo.atime = time(NULL);
    escribir_inodo(ninodo, &inodo);
    mi_signalSem();
    
    // Verificar permisos
    if ((inodo.permisos & 4) != 4)  {
       fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
       return FALLO;
    }

    // Preveer lectura más allá del fichero
    if (offset >= inodo.tamEnBytesLog) return read;
    if (offset + nbytes >= inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;

    // Lectura primer bloque
    unsigned char buf_bloque[BLOCKSIZE];
    int bf = traducir_bloque_inodo(ninodo, &inodo, primerBL, 0);
    if (primerBL == ultimoBL) { // el contenido leido cabe en un bloque
        if (bf != FALLO) {
            bread(bf, buf_bloque);
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        read += nbytes;
    } else { // hay maś bloques
        if (bf != FALLO) {
            bread(bf, buf_bloque);
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        read += BLOCKSIZE - desp1;
    

        // Lectura bloques intermedios
        for (int i = primerBL+1; i < ultimoBL; i++) {
            bf = traducir_bloque_inodo(ninodo, &inodo, i, 0);
            if (bf != FALLO) {
                bread(bf, buf_bloque);
                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL- 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            read += BLOCKSIZE;
        }

        // Lectura último bloque

        int desp2 = (offset + nbytes - 1) % BLOCKSIZE; // calcular desplazamiento
        bf = traducir_bloque_inodo(ninodo, &inodo, ultimoBL, 0);
        if (bf != FALLO) {
            bread(bf, buf_bloque);
            memcpy(buf_original + (nbytes - (desp2 + 1)), buf_bloque, desp2 + 1);
        }
        read += desp2 + 1;

    }
 
    return read;
} 



int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {
    // Leer inodo
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);

    // Guardar metainformación del fichero
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return EXITO;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {
    
    // Leer inodo
    struct inodo inodo;
    leer_inodo(ninodo,&inodo);
    mi_waitSem();

    // Modificar permisos
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    // Escribir el inodo modificado
    escribir_inodo(ninodo,&inodo);
    
    mi_signalSem();
    return EXITO;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    
    // Verificar permisos de escritura
    if ((inodo.permisos & 2) != 2)  {
        return FALLO; 
    }
    
    unsigned int tamaño_actual = inodo.tamEnBytesLog;
    
    // Verificar si se puede truncar más allá del tamaño actual
    if (nbytes >= tamaño_actual) {
        return FALLO;  
    }
    
    unsigned int primerBL;
    if (nbytes % BLOCKSIZE == 0)  primerBL = nbytes / BLOCKSIZE;       
    else primerBL = nbytes / BLOCKSIZE + 1;
    
    unsigned int bloques_liberados = liberar_bloques_inodo(primerBL, &inodo);    
    
    time_t tiempo_actual = time(NULL);
    
    inodo.mtime = tiempo_actual;
    inodo.ctime = tiempo_actual;
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - bloques_liberados;
    
    escribir_inodo(ninodo, &inodo);
    
    return bloques_liberados;
}
