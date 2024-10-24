/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/
#include "directorios.h"
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_stat <disco> </ruta>\n");
        return EXIT_FAILURE;
    }

    // Obtener los argumentos
    const char *disco = argv[1];
    const char *ruta = argv[2];

    // Montar dispositivo
    bmount(disco);
    
    // Estructura para almacenar información del archivo
    struct STAT stat_info;

    // Llamar a mi_stat para obtener la información del inodo
    int ninodo = mi_stat(ruta, &stat_info);
    if (ninodo < 0) {
        fprintf(stderr, RED "Error al obtener la información del inodo.\n" RESET);
        bumount();
        return EXIT_FAILURE;
    }

    fprintf(stdout, CYAN "Nº de inodo: %d\n" RESET, ninodo);
    fprintf(stdout, "tipo: %c\n", stat_info.tipo);
    fprintf(stdout, "permisos: %d\n", stat_info.permisos);
    //Mostrar tiempos
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    ts = localtime(&stat_info.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat_info.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat_info.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    fprintf(stdout, "atime: %s\nmtime: %s\nctime: %s\n",atime,mtime,ctime);
    fprintf(stdout, "nlinks: %d\n", stat_info.nlinks);
    fprintf(stdout, "tamEnBytesLog: %d\n", stat_info.tamEnBytesLog);
    fprintf(stdout, "numBloquesOcupados: %d\n", stat_info.numBloquesOcupados);

    bumount();
    
    return EXIT_SUCCESS;
}