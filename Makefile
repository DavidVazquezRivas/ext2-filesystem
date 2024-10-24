# Definición de variables
CC = gcc
CFLAGS = -c -g -Wall -std=gnu99 -I./include
LDFLAGS = -pthread

# Rutas de las carpetas
SRC_DIR = ./src
INCLUDE_DIR = ./include
CMDS_DIR = ./cmds
BUILD_DIR = ./build
BIN_DIR = ./bin

# Archivos fuente y de cabecera
SOURCES = $(SRC_DIR)/bloques.c $(SRC_DIR)/ficheros_basico.c $(SRC_DIR)/ficheros.c $(SRC_DIR)/directorios.c $(SRC_DIR)/semaforo_mutex_posix.c
LIBRARIES = $(BUILD_DIR)/bloques.o $(BUILD_DIR)/ficheros_basico.o $(BUILD_DIR)/ficheros.o $(BUILD_DIR)/directorios.o $(BUILD_DIR)/semaforo_mutex_posix.o
INCLUDES = $(INCLUDE_DIR)/bloques.h $(INCLUDE_DIR)/ficheros_basico.h $(INCLUDE_DIR)/ficheros.h $(INCLUDE_DIR)/directorios.h $(INCLUDE_DIR)/semaforo_mutex_posix.h $(INCLUDE_DIR)/simulacion.h $(INCLUDE_DIR)/verificacion.h

# Programas y sus fuentes
PROGRAMS = mi_mkfs leer_sf escribir leer permitir truncar mi_ls mi_mkdir mi_touch mi_chmod mi_stat mi_link mi_escribir mi_cat mi_rm mi_rmdir mi_escribir_varios prueba_cache_tabla simulacion verificacion
CMDS_SOURCES = $(addprefix $(CMDS_DIR)/, $(addsuffix .c, $(PROGRAMS)))

# Archivos objeto (usando la carpeta build)
OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.c=.o))) $(addprefix $(BUILD_DIR)/, $(notdir $(CMDS_SOURCES:.c=.o)))

# Regla principal
all: $(BIN_DIR) $(OBJS) $(addprefix $(BIN_DIR)/, $(PROGRAMS))

# Compilar los programas
$(BIN_DIR)/%: $(LIBRARIES) $(BUILD_DIR)/%.o
	$(CC) $(LDFLAGS) $(LIBRARIES) $(BUILD_DIR)/$*.o -o $@

# Compilar los archivos .o para src
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

# Compilar los archivos .o para cmds
$(BUILD_DIR)/%.o: $(CMDS_DIR)/%.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

# Crear las carpetas bin y build si no existen
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Limpiar
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*.o $(BIN_DIR)/* disco* ext*