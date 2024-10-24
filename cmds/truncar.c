/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "directorios.h"

int main(int argc, char **argv) {

    struct STAT stat;

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    // Comprobar sintaxis
    if (argc != 4) {
        perror(RED "No se han introducido los parametros correctos. Uso: ./truncar <nombre_dispositivo> <ninodo> <nbytes>" RESET);
        return FALLO;
    }
    
    // Montar disco
    char *nombre = argv[1];
    bmount(nombre);

    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);

    if (nbytes == 0) {
        liberar_inodo(ninodo);
    } else {
        mi_truncar_f(ninodo, nbytes);
    }

    mi_stat_f(ninodo, &stat);

    // Mostrar metainformación del inodo indicado
    printf("DATOS INODO %d\n", ninodo);
    printf("tipo = %c\n", stat.tipo);
    printf("permisos = %d\n", stat.permisos);
    ts = localtime(&stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("atime: %s\nmtime: %s\nctime: %s\n", atime, mtime, ctime);
    printf("nlinks = %d\n", stat.nlinks);
    printf("tamEnBytesLog = %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados = %d\n", stat.numBloquesOcupados);
    
    // Desmontar disco
    bumount();
}