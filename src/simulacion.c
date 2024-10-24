/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "../include/simulacion.h"

static int acabados = 0;

void reaper(){
  pid_t ended;
  signal(SIGCHLD, reaper);
  while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
     acabados++;
  }
}


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, RED "Sintaxis: ./simulacion <disco>\n" RESET);
        return FALLO;
    }

    // Asociar la señal SIGCHLD al enterrador
    signal(SIGCHLD, reaper);

    // Montar el dispositivo virtual (padre)
    char *nombre = argv[1];
    if (bmount(nombre) == -1) {
        perror("Error montando el dispositivo");
        return FALLO;
    }

    char temp[14];
    time_t tiempo;
    time(&tiempo);
    struct tm *tm;
    tm = localtime(&tiempo);
    sprintf(temp,"%d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
    char simul[8] = "/simul_";
    char *ruta = malloc(strlen(simul) + strlen(temp)+1);
    strcpy(ruta,simul);
    strcat(ruta,temp);
    strcat(ruta,"/");
    mi_creat(ruta,6);

    //printf("%s",ruta);

    pid_t pid;

    for(int i = 0; i < NUMPROCESOS; i++){
        pid = fork();
        if(pid == 0){
            bmount(nombre);

            char *aux = malloc(strlen(ruta) + sizeof(pid_t) + 21);
            strcpy(aux,simul);
            strcat(aux,temp);
            strcat(aux,"/");

            char proceso[9] = "proceso_";
            strcat(aux,proceso);

            char *auxiliar = malloc(sizeof(pid_t));
            sprintf(auxiliar,"%d",getpid());
            strcat(aux,auxiliar);
            strcat(aux,"/");

            mi_creat(aux,6);

            char ft[11] = "prueba.dat";
            strcat(aux,ft);

            int er = 0;
            mi_creat(aux,6);
            //printf(GREEN"Creamos el .dat\n"RESET);

            srand(time(NULL) + getpid());
            struct REGISTRO registro;
            for (int j = 1; j <= NUMESCRITURAS; j++) {
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = j;
                registro.nRegistro = rand() % REGMAX;

                er = mi_write(aux, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                if(er > 0){
                    #if DEBUGN12
                    printf(ORANGE"[simulacion.c → Escritura %d en %s]\n"RESET,j,aux);
                    #endif
                }
                usleep(50000); // Espera de 0.05 segundos (50,000 microsegundos)
            }

            if(er > 0){
                printf("[Proceso %d: Completadas %d escrituras en %s]\n", i+1, NUMESCRITURAS, aux);
            }
            
            bumount();
            return 0;
        }

        usleep(150000);
    }

    while(acabados < NUMPROCESOS){
        pause();
    }

    bumount();
    return 0;
}