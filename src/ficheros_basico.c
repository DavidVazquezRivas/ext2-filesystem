/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "../include/ficheros_basico.h" 

////////////////////////////////// NIVEL 2 ////////////////////////////////////

int tamMB(unsigned int nbloques) {
    int tamMB = (nbloques / 8) / BLOCKSIZE;
    if ((nbloques / 8) % BLOCKSIZE != 0) tamMB++;
    return tamMB;
} 

int tamAI(unsigned int nInodos) {
    int tamAI = (nInodos * INODOSIZE) / BLOCKSIZE;
    if ((nInodos * INODOSIZE) % BLOCKSIZE != 0) tamAI++;
    return tamAI;
}

int initSB(unsigned int nbloques, unsigned int nInodos){
    struct superbloque SB;
    // Inicializar superbloque
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(nInodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques-1;
    SB.posInodoRaiz = 0; 
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = nInodos;
    SB.totBloques = nbloques;
    SB.totInodos = nInodos;

    // Escribir bloque
    bwrite(posSB, &SB);

    return EXITO;
}

/* int initMB(unsigned int nbloques, unsigned int nInodos) {
    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);

    char bufferMB[BLOCKSIZE];
    memset(bufferMB, 0, BLOCKSIZE);
    unsigned int metaSize = tamSB + tamMB(nbloques) + tamAI(nInodos);

    // Escribir bloques completos para metadatos
    unsigned int nbloque = SB.posPrimerBloqueMB;
    while ((metaSize / BLOCKSIZE) / 8 > nbloque) {
        memset(bufferMB, 255, BLOCKSIZE);
        bwrite(nbloque, bufferMB);
        memset(bufferMB, 0, BLOCKSIZE);
        nbloque++;
    }

    // Escribir bloque incompleto
    unsigned int bytesRestantes = (metaSize / 8) % BLOCKSIZE;

    for (int i = 0; i < bytesRestantes; i++) {
        bufferMB[i] = 255; // 255: 11111111
    }
    unsigned int byteIncompleto = 0;
    for (int bitsRestantes = (metaSize % BLOCKSIZE) % 8; bitsRestantes > 0; bitsRestantes--) {
        byteIncompleto += power(2, 8 - bitsRestantes);
    }
    bufferMB[bytesRestantes] = byteIncompleto;

    bwrite(nbloque, bufferMB);

    // Restar bloques de metadatos a los bloques libres
    SB.cantBloquesLibres = SB.cantBloquesLibres - metaSize;

    // Escribir superbloque
    bwrite(posSB, &SB);

    return EXITO;
} */

int initMB() {
    struct superbloque SB;
    if (bread(posSB, &SB) == -1)
        return FALLO;

    char bufferMB[BLOCKSIZE];
    memset(bufferMB, 0, BLOCKSIZE);
    unsigned int metaSize = tamSB + tamMB(SB.totBloques) + tamAI(SB.totInodos);

    // Escribir bloques completos para metadatos
    unsigned int nbloque = SB.posPrimerBloqueMB;
    while ((metaSize / BLOCKSIZE) / 8 > nbloque) {
        memset(bufferMB, 255, BLOCKSIZE);
        bwrite(nbloque, bufferMB);
        memset(bufferMB, 0, BLOCKSIZE);
        nbloque++;
    }

    // Escribir bloque incompleto
    unsigned int bytesRestantes = (metaSize / 8) % BLOCKSIZE;

    for (int i = 0; i < bytesRestantes; i++) {
        bufferMB[i] = 255; // 255: 11111111
    }
    unsigned int byteIncompleto = 0;
    for (int bitsRestantes = (metaSize % BLOCKSIZE) % 8; bitsRestantes > 0; bitsRestantes--) {
        byteIncompleto += power(2, 8 - bitsRestantes);
    }
    bufferMB[bytesRestantes] = byteIncompleto;

    bwrite(nbloque, bufferMB);

    // Restar bloques de metadatos a los bloques libres
    SB.cantBloquesLibres = SB.cantBloquesLibres - metaSize;

    // Escribir superbloque
    bwrite(posSB, &SB);

    return EXITO;
} 

int power(int base, int exponent) {
    if (base == 0) return 0;
    int result = 1;
    for (int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

int initAI() {
    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // Inicializar inodos y escribirlos
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        bread(i, inodos);   // leer bloque de inodos
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            inodos[j].tipo = 'l';
            if (contInodos < SB.totInodos) {
                inodos[j].punterosDirectos[0] = contInodos; // enlazar siguiente inodo
                contInodos++;
            } else {    // ultimo inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }
        bwrite(i, inodos);  // escribir bloque de inodos
    }

    return EXITO;
}

////////////////////////////////// NIVEL 3 ////////////////////////////////////

int escribir_bit(unsigned int nbloque, unsigned int bit) {
    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);
    
    // Encontrar posición del bit
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueabs = posbyte / BLOCKSIZE + SB.posPrimerBloqueMB;
    posbyte = posbyte % BLOCKSIZE;
    
    // Leer mapa de bits
    unsigned char bufferMB[BLOCKSIZE];
    bread(nbloqueabs,bufferMB);
    
    // Escribir bit
    unsigned char mascara = 128;
    mascara >>= posbit;
    if (bit == 1){
        bufferMB[posbyte] |= mascara;
    } else if (bit == 0) {
        bufferMB[posbyte] &=~ mascara;
    }
    bwrite(nbloqueabs,bufferMB);
    
    return EXITO;
}

char leer_bit(unsigned int nbloque) {
    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);
    
    // Encontrar posición del bit
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueabs = posbyte / BLOCKSIZE + SB.posPrimerBloqueMB;
    posbyte = posbyte % BLOCKSIZE;
    
    // Leer mapa de bits
    unsigned char bufferMB[BLOCKSIZE];
    bread(nbloqueabs,bufferMB);
    
    // Leer bit
    unsigned char mascara = 128;
    mascara >>= posbit;           // desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha para dejar el 0 o 1 en el extremo derecho y leerlo en decimal
    
    return mascara;
}

int reservar_bloque() {
    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);

    // Verificar si hay bloques libres
    if (SB.cantBloquesLibres == 0) {
        perror("No hay bloques libres");
        return FALLO;
    }

    // Leer mapa de bits
    unsigned char bufferMB[BLOCKSIZE];
    bread(SB.posPrimerBloqueMB, bufferMB);

    // Crear bloque auxiliar   
    char bufferAux[BLOCKSIZE];
    memset(bufferAux, 255, BLOCKSIZE);
    
    // Buscar primer bloque MB con bits libres
    int nbloqueMB = 0;
    while (!memcmp(bufferAux, bufferMB, BLOCKSIZE) && (nbloqueMB < SB.posUltimoBloqueMB)) {
        nbloqueMB++;
        bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB);
    }

    // Buscar primer byte MB con bits libres
    // int posbyte = -1;
    // while (bufferMB[++posbyte] == UINT_MAX);

    int posbyte = 0;
    while ((bufferMB[posbyte] == 255) && (posbyte < BLOCKSIZE)) {
        posbyte++;
    }

    // Buscar primer bit MB libre
    unsigned char mascara = 128; //10000000
    int posbit = 0;
    while (bufferMB[posbyte] & mascara) {
        bufferMB[posbyte] <<= 1;
        posbit++;
    }   

    // Marcar el bloque como ocupado
    int nbloque = (nbloqueMB*BLOCKSIZE+posbyte)*8 + posbit;
    escribir_bit(nbloque,1);

    // Guardar superbloque
    SB.cantBloquesLibres--;
    bwrite(posSB, &SB);

    // Limpiar bloque en la zona de datos
    memset(bufferAux, 0, BLOCKSIZE);
    bwrite(nbloque, bufferAux);
    
    return nbloque;
} 


int liberar_bloque(unsigned int nbloque) {
    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);

    // Marcar bloque como libre en el mapa de bits
    escribir_bit(nbloque, 0);

    // Guardar superbloque
    SB.cantBloquesLibres++;
    bwrite(posSB, &SB);

    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo) {
    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);

    unsigned int nbloque = (ninodo / (BLOCKSIZE / INODOSIZE)) + SB.posPrimerBloqueAI;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];

    bread(nbloque, inodos);
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = *inodo;
    bwrite(nbloque, inodos);

    return EXITO;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo) {
    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);

    unsigned int nbloque = (ninodo / (BLOCKSIZE / INODOSIZE)) + SB.posPrimerBloqueAI;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    bread(nbloque, inodos);
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];

    return EXITO;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);

    if (SB.cantInodosLibres == 0) {
        perror("No hay inodos libres");
        return FALLO;
    }

    // Actualizar lista enlazada de inodos libres
    //struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // int nbloque = SB.posPrimerInodoLibre / (BLOCKSIZE / INODOSIZE) + SB.posPrimerBloqueAI;
    // bread(nbloque, inodos);
    int posInodoReservado = SB.posPrimerInodoLibre;
    struct inodo inodo;
    leer_inodo(posInodoReservado, &inodo);
    //inodo = inodos[posInodoReservado % (BLOCKSIZE / INODOSIZE)];
    SB.posPrimerInodoLibre = inodo.punterosDirectos[0];
    //inodo.punterosDirectos[0] = 0;

    // Inicializar campos del inodo
    inodo.tipo = tipo;
    inodo.permisos = permisos;
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.numBloquesOcupados = 0;
    inodo.atime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.mtime = time(NULL);
    
    for (int i = 0; i < DIRECTOS; i++)
        inodo.punterosDirectos[i] = 0;
    for (int i = 0; i < 3; i++)
        inodo.punterosIndirectos[i] = 0;

    // Escribir inodo
    escribir_inodo(posInodoReservado, &inodo);

    // Ajustar inodos libres
    SB.cantInodosLibres--;
    bwrite(posSB, &SB);

    return posInodoReservado;
}

////////////////////////////////// NIVEL 4 ////////////////////////////////////

int obtener_nRangoBL (struct inodo *inodo, unsigned int nblogico, unsigned int *ptr) {

    if (nblogico < DIRECTOS) { // <12
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }

    if (nblogico < INDIRECTOS0) { // <268
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }

    if (nblogico < INDIRECTOS1) { // <65.804
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }

    if (nblogico < INDIRECTOS2) { // 16.843.020
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }

    //Si se llega a este punto, estamos fuera de rango
    *ptr = 0;
    perror("Bloque lógico fuera de rango");
    return FALLO;
}

int obtener_indice(unsigned int nblogico, int nivel_punteros) {
    if (nblogico < DIRECTOS) { return nblogico; }
    if (nblogico < INDIRECTOS0) { return nblogico - DIRECTOS; }
    if (nblogico < INDIRECTOS1) {
        if (nivel_punteros == 2) { return (nblogico - INDIRECTOS0) / NPUNTEROS; }
        if (nivel_punteros == 1) { return (nblogico - INDIRECTOS0) % NPUNTEROS; }
        return FALLO;
    }
    if (nblogico < INDIRECTOS2) {
        if (nivel_punteros == 3) { return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);}
        if (nivel_punteros == 2) { return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS; }
        if (nivel_punteros == 1) { return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS; }
        return FALLO;
    }
    return FALLO;
}

int traducir_bloque_inodo(int ninodo, struct inodo *inodo, unsigned int nblogico, unsigned char reservar) {
    int modificado = 0;     // marca si se ha modificado el inodo
    unsigned int buffer[NPUNTEROS];
    unsigned int ptr = 0;
    unsigned int ptrAnt = 0;

    int nRangoBl = obtener_nRangoBL(inodo, nblogico, &ptr);
    int nivelPunteros = nRangoBl;
    int indice;
    // Hallar puntero directo a bloque de datos
    while (nivelPunteros > 0) {
        if (ptr == 0) { // se ha de crear el bloque de punteros
            if (reservar == 0) return FALLO; // bloque inexistente
            ptr = reservar_bloque(); // reservar bloque de inodos
            inodo->numBloquesOcupados++;
            inodo->ctime = time(NULL);
            modificado = 1;
            if (nivelPunteros == nRangoBl) {
                inodo->punterosIndirectos[nRangoBl - 1] = ptr;
                #if DEBUGN4
                fprintf(stderr, GRAY "[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %d (reservado BF %d para punteros_nivel%d)]\n" RESET, (nRangoBl - 1), ptr, ptr, nivelPunteros);
                #endif
            } else {
                buffer[indice] = ptr;
                bwrite(ptrAnt, buffer);
                #if DEBUGN4
                fprintf(stderr, GRAY "[traducir_bloque_inodo()→ punteros_nivel%d[%d] = %d (reservado BF %d para punteros_nivel%d)]\n" RESET, nivelPunteros + 1, indice, ptr, ptr, nivelPunteros);
                #endif
            }
            memset(buffer, 0, BLOCKSIZE);
        } else bread(ptr, buffer); // leer el bloque de punteros
        indice = obtener_indice(nblogico, nivelPunteros);
        ptrAnt = ptr;
        ptr = buffer[indice];
        nivelPunteros--;
    }

    // Crear bloque de datos si no existe
    if (ptr == 0) {
        if (reservar == 0) return FALLO; // bloque inexistente
        ptr = reservar_bloque(); // reservar bloque de datos
        inodo->numBloquesOcupados++;
        inodo->ctime = time(NULL);
        modificado = 1;
        if (nRangoBl == 0) { // es un puntero directo
            inodo->punterosDirectos[nblogico] = ptr;
            #if DEBUGN4
            fprintf(stderr, GRAY "[traducir_bloque_inodo()→ inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n" RESET, indice, ptr, ptr, nblogico);
            #endif
        } else {
            buffer[indice] = ptr;
            bwrite(ptrAnt, buffer); // escribir bloque de punteros
            #if DEBUGN4
            fprintf(stderr, GRAY "[traducir_bloque_inodo()→ punteros_nivel1[%d] = %d (reservado BF %d para BL %d)]\n" RESET, indice, ptr, ptr, nblogico);
            #endif
        }
    }

    // Escribir inodo si se ha modificado
    if (modificado) {
        escribir_inodo(ninodo, inodo);
    }
    return ptr;
} 

////////////////////////////////// NIVEL 6 ////////////////////////////////////

int liberar_inodo(unsigned int ninodo) {

    // Leer inodo
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);

    // Liberar bloques inodo
    int liberados = liberar_bloques_inodo(0, &inodo);

    // Actualizar inodo
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - liberados;
    if (inodo.numBloquesOcupados != 0){
        fprintf(stderr, RED "No se han liberado correctamente los bloques del inodo" RESET);
        return FALLO;
    }
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;
    inodo.ctime = time(NULL);

    // Actualizar lista enlazada de inodos libres
    struct superbloque SB;
    bread(posSB, &SB);
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;

    // Incrementar la cantidad de inodos libres en el superbloque
    SB.cantInodosLibres++;

    // Escribir el superbloque en el dispositivo
    bwrite(posSB, &SB);

    // Escribir inodo actualizado en el dispositivo virtual
    escribir_inodo(ninodo, &inodo);

    return ninodo;
}

int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo) {
    
    unsigned int nivel_punteros = 0, indice, ptr = 0, nBL, ultimoBL, breads = 0, bwrites = 0;
    int nRangoBL;
    unsigned int bloques_punteros[3][NPUNTEROS]; 
    unsigned int bufAux_punteros[NPUNTEROS]; 
    int ptr_nivel[3];
    int indices[3];
    int liberados = 0;

    if (inodo->tamEnBytesLog == 0) return liberados;

    ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0) ultimoBL--;

    #if DEBUGN6
    fprintf(stderr, CYAN BOLD "[liberar_bloques_inodo()→ primerBL: %d, último BL: %d]\n" RESET, primerBL, ultimoBL);
    #endif
    
    memset(bufAux_punteros, 0, BLOCKSIZE);
    
    for (nBL = primerBL; nBL <= ultimoBL; nBL++) {
        ptr = 0;
        nRangoBL= obtener_nRangoBL(inodo, nBL, &ptr);
        if (nRangoBL < 0) return FALLO;
        nivel_punteros = nRangoBL;

        while (ptr > 0 && nivel_punteros > 0) {
            indice = obtener_indice(nBL, nivel_punteros);
            if (indice == 0 || nBL == primerBL) {
                // Solo es necesario leer el bloque si no está ya guardado en un buffer
                bread(ptr, bloques_punteros[nivel_punteros - 1]);
                breads++;
            }
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if (ptr > 0) {
                liberar_bloque(ptr);
                #if DEBUGN6
                fprintf(stderr, GRAY "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n" RESET, ptr, nBL);
                #endif
                liberados++;

            if (nRangoBL == 0) {
                inodo->punterosDirectos[nBL] = 0;
            } else {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL) {
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0) {
                        // No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        liberados++;
                        #if DEBUGN6
                        fprintf(stderr, GRAY "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n" RESET, ptr, nivel_punteros, nBL);
                        #endif
                        // Saltar bloques que no sea necesario explorar al eliminar bloque de punteros
                        if (nBL < ultimoBL) {
                            unsigned int indices_restantes = NPUNTEROS - (indice + 1);
                            nBL += indices_restantes * power(NPUNTEROS, nivel_punteros - 1);
                            #if DEBUGN6
                            unsigned int antiguoBL = nBL;
                            fprintf(stderr, BLUE "[liberar_bloques_inodo()→ Del BL %d saltamos hasta BL %d]\n" RESET, antiguoBL, nBL);
                            #endif
                        }
                        
                        if (nivel_punteros == nRangoBL) {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;
                    } else { // Escribimos en el dispositivo el bloque de punteros modificado
                        bwrite(ptr, bloques_punteros[nivel_punteros - 1]);
                        bwrites++;
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        } else {
            // Saltar bloques que no hace falta leer al valer 0 el puntero
            if (nivel_punteros > 1) {
                nBL += power(NPUNTEROS, nivel_punteros);
                #if DEBUGN6
                unsigned int antiguoBL = nBL;
                fprintf(stderr, CYAN "[liberar_bloques_inodo()→ Del BL %d saltamos hasta BL %d]\n" RESET, antiguoBL, nBL);
                #endif
            }
        }
    }

    #if DEBUGN6
    fprintf(stderr, CYAN BOLD "[liberar_bloques_inodo()→ total bloques liberados: %d, total_breads: %d, total_bwrites: %d]\n" RESET, liberados, breads, bwrites);
    #endif
    return liberados;
}