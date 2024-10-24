/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "simulacion.h"

#define DEBUGN13 0

struct INFORMACION {
    int pid;
    unsigned int nEscrituras;//validadas
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
};

// Definición de la función format_time
void format_time(time_t epoch_time, char *buffer, size_t buffer_size) {
    struct tm *time_info = localtime(&epoch_time);
    strftime(buffer, buffer_size, "%a %d-%m-%Y %H:%M:%S", time_info);
}
 
