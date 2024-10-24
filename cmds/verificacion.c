/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "verificacion.h"

  int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, RED "%s <nombre_dispositivo> <directorio_simulación>\n", argv[0]);
        return FALLO;
    }

    // Montar disco
    char *nombre = argv[1];
    bmount(nombre);

    char *ruta_fichero = argv[2];
    // Verificar que es un directorio
    if(ruta_fichero[strlen(ruta_fichero)-1] != '/'){
        fprintf(stderr, RED "Error: no es un directorio. \n" RESET);
        return FALLO;
    }

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo, p_entrada;
    int resultado_busqueda = buscar_entrada(ruta_fichero, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);

    if (resultado_busqueda < 0) {
        mostrar_error_buscar_entrada(resultado_busqueda);
        bumount();
        return FALLO;
    }


    struct inodo inodo;
    leer_inodo(p_inodo, &inodo);

    struct STAT stat;
    mi_stat_f(p_inodo, &stat);

    int numentradas = stat.tamEnBytesLog / sizeof(struct entrada); // Aquí corregido

    if (numentradas != NUMPROCESOS) {
        bumount();
        return FALLO;
    }

    char informe_path[TAMNOMBRE];
    strcpy(informe_path, ruta_fichero);
    strcat(informe_path, "informe.txt");
    printf("ruta_informe = %s \n", informe_path);
    if ( mi_creat(informe_path, 6)!= EXITO ) {
        return FALLO;
    }
    struct entrada entradas[numentradas];
    printf("numero de entradas ->%ld \n",sizeof(entradas)/sizeof(struct entrada));

    if (mi_read(ruta_fichero, entradas, 0, sizeof(entradas)) == -1) {
        bumount();
        return FALLO;
    }
    int escrito = 0;
    
    // Procesar cada entrada en el directorio
    for (int i = 0; i < NUMPROCESOS; i++) {
        struct INFORMACION info;
        info.nEscrituras = 0;

        // Obtener la entrada actual
        struct entrada entrada;
        entrada = entradas[i];

        // Extraer el PID del nombre de la entrada
        char *pid_str = strchr(entrada.nombre, '_') + 1;
        pid_t pid = atoi(pid_str);

        info.pid = pid;
        // Construir la ruta completa del archivo "prueba.dat" sin usar snprintf
        char prueba_path[TAMNOMBRE];

        strcpy(prueba_path, ruta_fichero);
        strcat(prueba_path, entrada.nombre);
        strcat(prueba_path, "/prueba.dat");
        



        // Leer el contenido del archivo "prueba.dat" asociado al proceso
        int cant_registros_buffer_escrituras = 255;
        struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
    
        int offset = 0;    
        while (mi_read(prueba_path, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0) {
            for (int j = 0; j < cant_registros_buffer_escrituras; j++) {
                if (buffer_escrituras[j].pid == info.pid) { // escritura válida
                    if (info.nEscrituras == 0) {
                    info.MayorPosicion = buffer_escrituras[j];
                    info.MenorPosicion = buffer_escrituras[j];
                    info.PrimeraEscritura = buffer_escrituras[j];
                    info.UltimaEscritura = buffer_escrituras[j];
            } else {
                if (buffer_escrituras[j].nEscritura < info.PrimeraEscritura.nEscritura) {
                    info.PrimeraEscritura = buffer_escrituras[j];
                } else if (buffer_escrituras[j].nEscritura > info.UltimaEscritura.nEscritura) {
                    info.UltimaEscritura = buffer_escrituras[j];
                }
                if (buffer_escrituras[j].nRegistro < info.MenorPosicion.nRegistro) {
                    info.MenorPosicion = buffer_escrituras[j];
                } else if (buffer_escrituras[j].nRegistro > info.MayorPosicion.nRegistro) {
                    info.MayorPosicion = buffer_escrituras[j];
                }
            }
            info.nEscrituras++;
        }
    }
    memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
    offset += sizeof(buffer_escrituras);
}

   // Buffers para las fechas formateadas
        char fecha_primera[TAMNOMBRE];
        char fecha_ultima[TAMNOMBRE];
        char fecha_menor[TAMNOMBRE];
        char fecha_mayor[TAMNOMBRE];

        // Formatear las fechas usando strftime
        strftime(fecha_primera, TAMNOMBRE, "%a %d-%m-%Y %H:%M:%S", localtime(&info.PrimeraEscritura.fecha));
        strftime(fecha_ultima, TAMNOMBRE, "%a %d-%m-%Y %H:%M:%S", localtime(&info.UltimaEscritura.fecha));
        strftime(fecha_menor, TAMNOMBRE, "%a %d-%m-%Y %H:%M:%S", localtime(&info.MenorPosicion.fecha));
        strftime(fecha_mayor, TAMNOMBRE, "%a %d-%m-%Y %H:%M:%S", localtime(&info.MayorPosicion.fecha));

    // Imprimir la información en el formato solicitado
    char buffer[TAMNOMBRE * 10];
        int len = snprintf(buffer, sizeof(buffer),
        "PID: %d\n"
        "Numero de escrituras: %u\n"
        "Primera Escritura    %10d    %10d    %s\n"
        "Ultima Escritura     %10d    %10d    %s\n"
        "Menor Posición       %10d    %10d    %s\n"
        "Mayor Posición       %10d    %10d    %s\n",
        info.pid, info.nEscrituras,
        info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, fecha_primera,
        info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, fecha_ultima,
        info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, fecha_menor,
        info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, fecha_mayor
    );

        #if DEBUGN13
        printf("Validadas %d escrituras en %s \n",info.nEscrituras,prueba_path);
        #endif
        escrito += mi_write(informe_path, buffer, escrito, len);

    }

    
    // Desmontar disco
    bumount();

    return EXITO;
}
