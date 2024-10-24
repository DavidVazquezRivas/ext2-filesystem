/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "../include/directorios.h"

// tabla caché directorios
#if USARCACHE == 1
    static struct UltimaEntrada UltimaEntradaEscritura;
#elif (USARCACHE == 2 || USARCACHE == 3)
   #define CACHE_SIZE 3 // cantidad de entradas para la caché
   static struct UltimaEntrada UltimasEntradas[CACHE_SIZE];
#endif

void imprimir_cadena_con_no_visibles(const char *cadena) {
    while (*cadena != '\0') {
        if (*cadena < 32 || *cadena == 127) { // Caracteres no visibles o DEL (127)
            printf("\\x%02X", (unsigned char)*cadena);
        } else {
            putchar(*cadena);
        }
        cadena++;
    }
    printf("\n");
}

/******************************************************************************
 * NIVEL 7
 *****************************************************************************/

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    if(camino[0] != '/') {
        return FALLO;
    }

    const char *segunda_barra = strchr(camino + 1, '/');

    if (!segunda_barra) {
        strcpy(inicial, camino + 1);
        *tipo = 'f';
        strcpy(final, "");
    } else {
        strncpy(inicial, camino + 1, segunda_barra - camino - 1);
        inicial[segunda_barra - camino - 1] = '\0';
        *tipo = 'd';
        strcpy(final, segunda_barra);
    }

    return EXITO;
}

 int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    // Declaraciones
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);

    // Caso inodo raiz
    if (!strcmp(camino_parcial, "/")) {
        *p_inodo = 0;
        *p_entrada = 0;
        return EXITO;
    }

    // Caso camino incorrecto
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO) return ERROR_CAMINO_INCORRECTO;

    #if DEBUGN7
    fprintf(stderr, GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n" RESET, inicial, final, reservar);
    #endif

    // Buscar entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4)  return ERROR_PERMISO_LECTURA;
    
    unsigned int entradas_por_bloque = BLOCKSIZE / sizeof(struct entrada);
    struct entrada buff_entradas[entradas_por_bloque];
    memset(buff_entradas, 0, BLOCKSIZE);

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;
    int offset = 0;
    if (cant_entradas_inodo > 0) {
        offset += mi_read_f(*p_inodo_dir, buff_entradas, offset, BLOCKSIZE);
        int i = 0;
        entrada = buff_entradas[i++];
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entrada.nombre))) {
            num_entrada_inodo++;
            entrada = buff_entradas[i++];
            if (i == entradas_por_bloque) {
                // Volver a leer un buffer de entradas del disco
                memset(buff_entradas, 0, BLOCKSIZE);
                offset += mi_read_f(*p_inodo_dir, buff_entradas, offset, BLOCKSIZE);
                i = 0;
            }
        }
    }

    // Caso en que la entrada no existe
    if (strcmp(inicial, entrada.nombre) && num_entrada_inodo == cant_entradas_inodo) {
        // Modo consulta
        if (!reservar) return ERROR_NO_EXISTE_ENTRADA_CONSULTA;

        // Modo escritura
        // Crear entrada en el directorio padre
        if (inodo_dir.tipo == 'f') return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;

        if ((inodo_dir.permisos & 2) != 2) return ERROR_PERMISO_ESCRITURA;

        if (tipo == 'd') {
            if (strcmp(final, "/")) return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;

            strcpy(entrada.nombre, inicial);
            entrada.inodo = reservar_inodo('d', permisos);
            #if DEBUGN7
            fprintf(stderr, GRAY "[buscar_entrada()→ reservado inodo %d tipo d con permisos %d para %s]\n" RESET, entrada.inodo, permisos, entrada.nombre);
            #endif
        } else {
            strcpy(entrada.nombre, inicial);
            entrada.inodo = reservar_inodo('f', permisos);
            #if DEBUGN7
            fprintf(stderr, GRAY "[buscar_entrada()→ reservado inodo %d tipo f con permisos %d para %s]\n" RESET, entrada.inodo, permisos, entrada.nombre);
            #endif
        }

        // Escribir entrada en el directorio padre
        if (mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(struct entrada)) != sizeof(struct entrada)) {
            if (entrada.inodo != -1) {
                liberar_inodo(entrada.inodo);
            }
            return FALLO;
        }
        // Volver a leer el directorio padre pues este puede haber cambiado
        leer_inodo(*p_inodo_dir, &inodo_dir);
        #if DEBUGN7
        fprintf(stderr, GRAY "[buscar_entrada()→ creada entrada: %s, %d]\n" RESET, entrada.nombre, entrada.inodo);
        #endif
    }

    if (!strcmp(final, "") || !strcmp(final, "/")) { // Hemos llegado al final del camino
        if (num_entrada_inodo < cant_entradas_inodo && reservar == 1) return ERROR_ENTRADA_YA_EXISTENTE;

        *p_inodo = entrada.inodo;
        *p_entrada = num_entrada_inodo;
    } else {
        *p_inodo_dir = entrada.inodo;
        *p_inodo = 0;
        *p_entrada = 0;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return EXITO;
}
/*int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada,
                   char reservar, unsigned char permisos) {
    struct inodo inodo_dir;
    struct entrada entrada;

    int cant_entradas_inodo, num_entrada_inodo=0; //cantidad total de entradas del inodo y nº de la que estamos analizando
 
    int encontrada=0;

    char tipo;
    // Inicializamos variables con el tamaño del campo nombre de la struct entrada
    char inicial[sizeof(entrada.nombre)];  
    // char *inicial; inicial = (char *)malloc(sizeof(entrada.nombre));
    char final[strlen(camino_parcial)+1]; 
    // char *final; final = (char *)malloc(strlen(camino_parcial));
    

    if (strcmp(camino_parcial, "/") == 0) {//si es el directorio raíz
        // fprintf(stderr, MAGENTA "camino=/\n" RESET);
        *p_inodo_dir = 0;
      *p_inodo = 0; // la raiz siempre estará asociada al inodo 0
      *p_entrada = 0;
      return EXITO;
    } 

    // Limpiamos inicial y final
    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial)+1);
    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0)
        return ERROR_CAMINO_INCORRECTO;
    #if DEBUGN7
    fprintf (stderr, GREEN "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n" RESET, inicial, final, reservar);
    #endif

    if (leer_inodo(*p_inodo_dir, &inodo_dir) == -1) return FALLO;
    if ((inodo_dir.permisos & 4) != 4) { 
        #if DEBUGN7    
        fprintf(stderr, GREEN "[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n" RESET, *p_inodo_dir);
        #endif
        return ERROR_PERMISO_LECTURA;
    }

    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    //fprintf(stderr, GREEN "[buscar_entrada()→ El inodo %d tiene %d entradas]\n" RESET, *p_inodo_dir, cant_entradas_inodo);
    if (cant_entradas_inodo > 0) {

        //bucle leyendo entrada a entrada del disco
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1) { //Se lee la primera entrada
            fprintf(stderr, "Error: directorios.c → buscar_entrada() → mi_read_f(*p_inodo_dir, &entrada, 0, sizeof(struct entrada)).\n");
            return -1;
        }
        //fprintf(stderr,"num_entrada_inodo: %d\n", num_entrada_inodo);
        //fprintf(stderr,"entrada.nombre: %s\n", entrada.nombre);
        //fprintf(stderr,"inicial: %s\n", inicial);

        // buscamos la entrada cuyo nombre se encuentra en inicial
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(entrada.nombre, inicial) != 0)) {
            num_entrada_inodo++;
            //Leer siguiente entrada.
            memset(entrada.nombre, 0, sizeof(entrada.nombre));
            if (mi_read_f(*p_inodo_dir, &entrada, (num_entrada_inodo * sizeof(struct entrada)), sizeof(struct entrada)) == -1) {
                fprintf(stderr, "Error: directorios.c → buscar_entrada() → mi_read_f(*p_inodo_dir, &entrada, (num_entrada_inodo * sizeof(struct entrada)), sizeof(struct entrada))\n");
                return -1;
            }
            //fprintf(stderr,"num_entrada_inodo: %d\n", num_entrada_inodo);
            //fprintf(stderr,"entrada.nombre: %s\n", entrada.nombre);
            *p_entrada=num_entrada_inodo;
        }  
        //fin bucle leyendo entrada a entrada del disco 
    
    }
    //fprintf(stderr, MAGENTA "entrada %s encontrada: %d\n" RESET, entrada.nombre, encontrada);
    if ((num_entrada_inodo == cant_entradas_inodo) && !encontrada) { // la entrada no existe
        switch (reservar) {
        case 0: // modo consulta. Como no existe retornamos error
            *p_entrada = cant_entradas_inodo;
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1: // modo escritura
            if (inodo_dir.tipo == 'f') { // no podemos crear entradas dentro de un fichero
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            if ((inodo_dir.permisos & 2) != 2) { //si no tiene permiso de escritura
               return ERROR_PERMISO_ESCRITURA;
            } else {
                strcpy(entrada.nombre, inicial);
                if (tipo == 'd' && strcmp(final, "/") != 0) return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                entrada.inodo = reservar_inodo(tipo == 'd' ? 'd' : 'f', permisos);
                #if DEBUGN7
                fprintf (stderr, GREEN "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n" RESET, 
                entrada.inodo, tipo, permisos, inicial);
                #endif
                // Creamos la entrada en el directorio referenciado por *p_inodo_dir
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1) {
                    liberar_inodo(entrada.inodo);
                    fprintf(stderr, RED "Error: directorios.c → buscar_entrada() → mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada))\n" RESET);
                    return FALLO;
                } else {
                    #if DEBUGN7
                    fprintf (stderr, GREEN "[buscar_entrada()→ creada entrada: %s, %d]\n" RESET, entrada.nombre, entrada.ninodo);
                    #endif
                }
            }
        }
    }
    //determinar si hemos de seguir la recursividad o cortarla
    if (strcmp(final, "/") == 0 || strcmp(final, "") == 0) { // hemos llegado al final del camino
        if (encontrada && reservar == 1)     // modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        // cortamos la recursividad
        *p_inodo = entrada.inodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    } else {//lamada recursiva
        *p_inodo_dir = entrada.inodo;
        *p_inodo = 0;
        *p_entrada = 0;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    } 
    return EXITO;
}*/





void mostrar_error_buscar_entrada(int error) {
    switch (error) {
        case -2: fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET); break;
        case -3: fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET); break;
        case -4: fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET); break;
        case -5: fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" RESET); break;
        case -6: fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" RESET); break;
        case -7: fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET); break;
        case -8: fprintf(stderr, RED "Error: No es un directorio.\n" RESET); break;
    }
}


/******************************************************************************
 * NIVEL 8
 *****************************************************************************/


int mi_dir(const char *camino, char *buffer, char tipo, int modo) {
    unsigned int p_inodo;
    unsigned int p_entrada;
    unsigned int p_inodo_dir = 0;

    struct entrada buff_entradas[TAMBUFFER];
    char bufAux[TAMBUFFER];
    memset(bufAux, 0, TAMBUFFER);
    
    unsigned int entradas_leidas = 0;

    // Leer entrada directorio
    int resultado = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (resultado != EXITO) {
        mostrar_error_buscar_entrada(resultado);
        return FALLO;
    }

    struct entrada entrada;
    if (p_inodo == p_inodo_dir) { // Directorio raiz
        entrada.inodo = p_inodo;
        strcpy(entrada.nombre, "/");
    } else {
        mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada));
    }
    
    // Leer inodo para hacer comprobaciones
    struct inodo inodo;
    leer_inodo(entrada.inodo, &inodo);

    if (tipo != inodo.tipo) {
        fprintf(stderr, RED "Error: la sintaxis no concuerda con el tipo\n" RESET);
        return FALLO;
    }

    if ((inodo.permisos & 4) != 4) {
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET);
        return FALLO;
    }
    
    if (tipo == 'f') { // Se lee un fichero
        leer_datos_entrada(buffer, entrada, 1);
        entradas_leidas++;
    } else { // Se lee un directorio
        // Recorrer las entradas del directorio
        int read = mi_read_f(entrada.inodo, buff_entradas, 0, TAMBUFFER);
        while(read > 0) {
            leer_datos_entrada(buffer, buff_entradas[entradas_leidas % TAMFILA], modo);
            
            entradas_leidas++;
            read -= sizeof(struct entrada);
            // Leer siguientes entradas en buffer
            if (read == 0) {
                read = mi_read_f(entrada.inodo, buff_entradas, (entradas_leidas) * sizeof(struct entrada), TAMBUFFER);
            }
        }
    }

    return entradas_leidas;
}

void leer_datos_entrada(char *buffer, struct entrada entrada, int modo) {
    // Leer inodo
    struct inodo inodo;
    leer_inodo(entrada.inodo, &inodo);

    if (modo) { // Modo será 0 si no se han de mostrar datos detallados
        // Añadir tipo
        char temp1[2] = {inodo.tipo, '\0'};
        strcat(buffer, temp1);
        strcat(buffer, "\t");

        // Añadir permisos
        if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
        if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
        if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
        strcat(buffer, "\t\t");

        // Añadir fecha y hora
        struct tm *tm;
        tm = localtime(&(inodo.mtime));
        char tmp[30];
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,  tm->tm_sec);
        strcat(buffer, tmp);

        // Añadir tamaño
        char temp2[12]; // Mínimo tamaño para contener un entero y el carácter de terminación
        sprintf(temp2, "%d", inodo.tamEnBytesLog); // Convertir el tamaño a una cadena de caracteres
        strcat(buffer, temp2);
        strcat(buffer, "\t\t");

        // Añadir nombre
        const char* color = (inodo.tipo == 'd') ? ORANGE : CYAN;
        sprintf(buffer + strlen(buffer), "%s%s\n" RESET, color, entrada.nombre);
    } else {
        const char* color = (inodo.tipo == 'd') ? ORANGE : CYAN;
        sprintf(buffer + strlen(buffer), "%s%s\t" RESET, color, entrada.nombre);
    }
    
}

int mi_chmod(const char *camino, unsigned char permisos){
    unsigned int p_inodo;
    unsigned int p_entrada;
    unsigned int p_inodo_dir = 0;
    int resultado_busqueda = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);

    if (resultado_busqueda == EXITO){
        if (mi_chmod_f(p_inodo, permisos) != EXITO){
            fprintf(stderr, RED "Error al cambiar los permisos del archivo\n" RESET);
            return FALLO;
        }
        return EXITO;
    }

    mostrar_error_buscar_entrada(resultado_busqueda);
    return resultado_busqueda;
}

int mi_stat(const char *camino, struct STAT *p_stat) {
    unsigned int p_inodo;
    unsigned int p_entrada;
    unsigned int p_inodo_dir = 0;

    // Buscar la entrada de camino para obtener el número de inodo
    int resultado_busqueda = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);

    if (resultado_busqueda == EXITO) {
        // Llamar a mi_stat_f de ficheros.c pasando el número de inodo
        if (mi_stat_f(p_inodo, p_stat) != EXITO) {
            fprintf(stderr, RED "Error al obtener la información del archivo.\n" RESET);
            return -1;
        }
        return p_inodo;
    }

    // Mostrar error si la entrada no se encuentra
    mostrar_error_buscar_entrada(resultado_busqueda);
    return resultado_busqueda;
}

int mi_creat(const char *camino, unsigned char permisos) {
    mi_waitSem();

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo, p_entrada;

    // char *auxiliar = malloc(strlen(camino));
    // strcpy(auxiliar,camino);

    int aux = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);

    mostrar_error_buscar_entrada(aux);

    mi_signalSem();
    return aux;
}

/******************************************************************************
 * NIVEL 9
 *****************************************************************************/

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo, p_entrada;
    char reservar = 0;
    unsigned char permisos = 6;
    int aux;

    #if USARCACHE == 0
    aux = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, permisos);
    if (aux < 0) {
        mostrar_error_buscar_entrada(aux);
        return FALLO;
    }
    #else 

    int uso_cache = 0;

    #if USARCACHE == 1
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0) {
        p_inodo = UltimaEntradaEscritura.p_inodo;
    } else {
        uso_cache = 0; // Se mantiene a 0, ya que su función es comprobar si la entrada se encuentra en la caché
    }
    #elif USARCACHE == 2 || USARCACHE == 3
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (strcmp(UltimasEntradas[i].camino, camino) == 0) {
            p_inodo = UltimasEntradas[i].p_inodo;
            uso_cache = 1;
            #if USARCACHE == 3
            gettimeofday(&UltimasEntradas[i].ultima_consulta, NULL);
            #endif
            #if DEBUGCACHE2 || DEBUGCACHE3
            //printf( CYAN "[mi_write() → Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n" RESET);
            printf(CYAN"mi_write() → Utilizamos cache[%d] : %s\n"RESET,i,UltimasEntradas[i].camino);
            #endif
            break; // Salir del bucle una vez encontrada la entrada
        }
    }
    #endif

    if (!uso_cache) {
        aux = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, permisos);
        mostrar_error_buscar_entrada(aux);
        #if USARCACHE > 1
        actualizar_cache(camino, p_inodo);
        #endif
    }
    #endif

    return mi_write_f(p_inodo, buf, offset, nbytes);
}

void actualizar_cache(const char *camino, unsigned int p_inodo) {
#if USARCACHE == 2 // FIFO
    static int ultimo = 0;
    #if DEBUGCACHE2
    printf(ORANGE"mi_write() → Reemplazamos cache[%d] : %s\n"RESET,ultimo,camino);
    #endif
    //printf(ORANGE"[mi_write() → Actualizamos la caché de escritura]\n"RESET);
    strcpy(UltimasEntradas[ultimo].camino, camino);
    UltimasEntradas[ultimo].p_inodo = p_inodo;
    ultimo = (ultimo + 1) % CACHE_SIZE;
#endif

#if USARCACHE == 3 // LRU
    int lru = 0;
    struct timeval minimo = UltimasEntradas[0].ultima_consulta;

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (timercmp(&UltimasEntradas[i].ultima_consulta, &minimo, <)) {
            lru = i;
            minimo = UltimasEntradas[i].ultima_consulta;
        }
    }
    #if DEBUGCACHE3
    printf(ORANGE"mi_write() → Reemplazamos cache[%d] : %s\n"RESET,lru,camino);
    //printf(ORANGE"[mi_write() → Actualizamos la caché de escritura]\n"RESET);
    #endif

    strcpy(UltimasEntradas[lru].camino, camino);  // Copia el camino al elemento de la caché
    UltimasEntradas[lru].p_inodo = p_inodo;       // Actualiza el inodo
    gettimeofday(&UltimasEntradas[lru].ultima_consulta, NULL);  // Actualiza el tiempo de consulta
#endif
}


int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo, p_entrada;
    int resultado_busqueda = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (resultado_busqueda < 0) {
        mostrar_error_buscar_entrada(resultado_busqueda);
        return FALLO;
    }

    int bytesleidos = mi_read_f(p_inodo, buf, offset, nbytes);

    return bytesleidos;
}

/******************************************************************************
 * NIVEL 10
 *****************************************************************************/

int mi_link(const char *camino1, const char *camino2) {
    mi_waitSem();

    unsigned int p_inodo_dir1 = 0;
    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo1, p_entrada1, p_inodo2, p_entrada2;
    int resultado1, resultado2;

    // Obtener inodo referido (1)
    resultado1 = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 0);
    if (resultado1 < 0) {
        mostrar_error_buscar_entrada(resultado1);
        mi_signalSem();
        return FALLO;
    } else if (p_inodo1 == p_inodo_dir1) {
        fprintf(stderr, RED "Error: No se puede enlazar un directorio (es el directorio raiz)\n" RESET);
        mi_signalSem();
        return FALLO;
    }
    struct entrada entrada1;
    mi_read_f(p_inodo_dir1, &entrada1, p_entrada1 * sizeof(struct entrada), sizeof(struct entrada));
    struct inodo inodo;
    leer_inodo(entrada1.inodo, &inodo);

    if (!(inodo.permisos & 4)) {
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    if (inodo.tipo != 'f') {
        fprintf(stderr, RED "Error: No se puede enlazar un directorio\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Crear enlace
    resultado2 = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if (resultado2 < 0) {
        mostrar_error_buscar_entrada(resultado2);
        mi_signalSem();
        return FALLO;
    }
    struct entrada entrada2;
    mi_read_f(p_inodo_dir2, &entrada2, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada));
    entrada2.inodo = entrada1.inodo;
    // Reescribir la nueva entrada
    mi_write_f(p_inodo_dir2, &entrada2, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada));
    // Liberar inodo creado para la nueva entrada
    liberar_inodo(p_inodo2);

    // Modificar el inodo y guardarlo
    inodo.nlinks++;
    inodo.ctime = time(NULL);
    escribir_inodo(entrada1.inodo, &inodo);

    mi_signalSem();
    return EXITO;
}

int mi_unlink(const char *camino) {
    mi_waitSem();

    unsigned int p_inodo;
    unsigned int p_entrada;
    unsigned int p_inodo_dir = 0;

    int resultado_busqueda = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);

    if (resultado_busqueda < 0) {
        mostrar_error_buscar_entrada(resultado_busqueda);
        mi_signalSem();
        return FALLO;
    } else if (p_inodo == p_inodo_dir) {
        fprintf(stderr, RED "Error: no se puede eliminar el directorio raíz" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Leer inodo
    struct inodo inodo;
    leer_inodo(p_inodo, &inodo);
    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0) {
        fprintf(stderr, RED "Error: El directorio %s no está vacío\n" RESET, camino);
        mi_signalSem();
        return FALLO;
    }

    // Calcular entradas del directorio padre
    struct inodo inodo_padre;
    leer_inodo(p_inodo_dir, &inodo_padre);
    int nEntradas = inodo_padre.tamEnBytesLog / sizeof(struct entrada);
    
    // Eliminar entrada
    if (p_entrada != nEntradas - 1) {
        // Leer ultima entrada y sobreescribir la entrada a borrar
        struct entrada ultima_entrada;
        mi_read_f(p_inodo_dir, &ultima_entrada, (nEntradas - 1) * sizeof(struct entrada), sizeof(struct entrada));
        mi_write_f(p_inodo_dir, &ultima_entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada));
    }
    mi_truncar_f(p_inodo_dir, (nEntradas - 1) * sizeof(struct entrada));

    // Decrementar número de enlaces del inodo y eliminar si es necesario
    inodo.nlinks--;
    if (!inodo.nlinks) {
        liberar_inodo(p_inodo);
    } else {
        inodo.ctime = time(NULL);
        escribir_inodo(p_inodo, &inodo);
    }

    mi_signalSem();
    return EXITO;
}