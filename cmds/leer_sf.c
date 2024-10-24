/*
Pau Antich, Nasim Benyacoub y David Vázquez
*/

#include "directorios.h"

void mostrarSB(struct superbloque SB);
void mostrarAI(struct superbloque SB);
void mostrarMB(struct superbloque SB);
void mostrarInodo(unsigned int posInodo);
void mostrar_buscar_entrada(char *camino, char reservar);


int main(int argc, char** argv) {
    
    // Comprobar sintaxis
    if (argc != 2) {
        perror(RED "No se han introducido los parametros correctos. Uso: ./leer_sf <nombre_disco>" RESET);
        return FALLO;
    }

    // Comprobar que el disco existe
    if (access(argv[1], F_OK) == -1) {
        perror("El disco no existe");
        return FALLO;
    }

    // Montar disco
    char *nombre = argv[1];
    bmount(nombre);

    // Leer superbloque
    struct superbloque SB;
    bread(posSB, &SB);


    // Mostrar datos superbloque
    mostrarSB(SB);
    
    #if DEBUGN2
    // Mostrar tamaños de superbloque e inodo
    printf("sizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n\n", sizeof(struct inodo));

    // Mostrar array de inodos
    mostrarAI(SB);
    #endif

    #if DEBUGN3
    // Reservar bloque y luego liberarlo
    printf("\n\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS");
    unsigned int nbloque = reservar_bloque();
    bread(posSB, &SB);
    printf("Se ha reservado el bloque física nº %d que era el 1º libre indicado por el MB\n", nbloque);
    printf("SB.cantBloquesLibres = %d \n", SB.cantBloquesLibres);
    liberar_bloque(nbloque);
    bread(posSB, &SB);
    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);

    // Mostrar el MB
    mostrarMB(SB);

    // Mostrar el directorio raíz
    printf("\n\nDATOS DEL DIRECTORIO RAIZ\n");
    mostrarInodo(SB.posInodoRaiz);

    #endif

    #if DEBUGN3
    int ninodo = reservar_inodo('f', 6);
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    printf("INODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n\n");
    fflush(stdout);
    traducir_bloque_inodo(ninodo, &inodo, 8, 1);
    fprintf(stderr, "\n\n");
    traducir_bloque_inodo(ninodo, &inodo, 204, 1);
    fprintf(stderr, "\n\n");
    traducir_bloque_inodo(ninodo, &inodo, 30004, 1);
    fprintf(stderr, "\n\n");
    traducir_bloque_inodo(ninodo, &inodo, 400004, 1);
    fprintf(stderr, "\n\n");
    traducir_bloque_inodo(ninodo, &inodo, 468750, 1);

    mostrarInodo(ninodo);
    #endif

    #if DEBUGN7
    //Mostrar creación directorios y errores
    mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  
    //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
    #endif

    // Desmontar disco
    bumount();
}

void mostrarSB(struct superbloque SB) {
    printf("DATOS DEL SUPERBLOQUE:\n");
    printf("posPrimerBloqueMB: %d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB: %d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI: %d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI: %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos: %d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos: %d\n",SB.posUltimoBloqueDatos);
    printf("posInodoRaiz: %d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre: %d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres: %d\n", SB.cantBloquesLibres);
    printf("cantInodosLibres: %d\n", SB.cantInodosLibres);
    printf("totBloques: %d\n", SB.totBloques);
    printf("totInodos: %d\n", SB.totInodos);
}

void mostrarAI(struct superbloque SB) {
    printf("RECORRIDO LISTA ENLAZADA DE INODOS\n");
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        bread(i, inodos);   // leer bloque de inodos
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            printf("%d ",inodos[j].punterosDirectos[0]);
        }
    }
}

void mostrarMB(struct superbloque SB) {
    printf("\n\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    printf("posSB: %d → leer_bit(%d) = %d\n",posSB,posSB,leer_bit(posSB));
    printf("SB.posPrimerBloqueMB: %d → leer_bit(%d) = %d\n",SB.posPrimerBloqueMB,SB.posPrimerBloqueMB,leer_bit(SB.posPrimerBloqueMB));
    printf("SB.posUltimoBloqueMB: %d → leer_bit(%d) = %d\n",SB.posUltimoBloqueMB,SB.posUltimoBloqueMB,leer_bit(SB.posUltimoBloqueMB));
    printf("SB.posPrimerBloqueAI: %d → leer_bit(%d) = %d\n",SB.posPrimerBloqueAI,SB.posPrimerBloqueAI,leer_bit(SB.posPrimerBloqueAI));
    printf("SB.posUltimoBloqueAI: %d → leer_bit(%d) = %d\n",SB.posUltimoBloqueAI,SB.posUltimoBloqueAI,leer_bit(SB.posUltimoBloqueAI));
    printf("SB.posPrimerBloqueDatos: %d → leer_bit(%d) = %d\n",SB.posPrimerBloqueDatos,SB.posPrimerBloqueDatos,leer_bit(SB.posPrimerBloqueDatos));
    printf("SB.posUltimoBloqueDatos: %d → leer_bit(%d) = %d\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));
}

void mostrarInodo(unsigned int ninodo) {
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %d\n", inodo.permisos);

    // Mostrar tiempos
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("atime: %s\nmtime: %s\nctime: %s\n",atime,mtime,ctime);

    printf("nlinks: %d\n", inodo.nlinks);
    printf("tamEnBytesLog: %d\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inodo.numBloquesOcupados);
}

void mostrar_buscar_entrada(char *camino, char reservar) {
  unsigned int p_inodo_dir = 0;
  unsigned int p_inodo = 0;
  unsigned int p_entrada = 0;
  int error;
  printf("\ncamino: %s, reservar: %d\n", camino, reservar);
  if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
    mostrar_error_buscar_entrada(error);
  }
  printf("**********************************************************************\n");
  return;
}
