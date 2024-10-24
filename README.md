# ext2-filesystem

## 🌐 Introducción

Este proyecto implementa un sistema de ficheros basado en **ext2**, un componente fundamental de muchos sistemas operativos Unix y Linux. Aprender sobre sistemas de ficheros es crucial para entender cómo se gestionan los datos en los dispositivos de almacenamiento y cómo se optimizan para el rendimiento y la seguridad. A través de esta implementación, se profundiza en la gestión de inodos, la asignación de espacio y el control de concurrencia, ofreciendo una base sólida para cualquier estudiante o desarrollador que desee comprender mejor los sistemas operativos.

## 🔑 Características principales

- **Sistema jerárquico:** Crea directorios, ficheros y enlaces físicos, similar a la estructura de un grafo.
- **Asignación dinámica de espacio:** Los bloques de datos se asignan según las necesidades, soportando tamaños de bloque entre 512 y 4096 bytes.
- **Gestión de inodos:** Utiliza inodos de 128 bytes con 12 punteros directos, y punteros indirectos simples, dobles y triples.
- **Primitivas básicas:** Comandos como `mi_ls`, `mi_chmod`, `mi_cat`, `mi_link`, `mi_rm`, y `mi_stat` replican el comportamiento de utilidades de GNU/Linux (`ls`, `chmod`, `cat`, `ln`, `rm`, `stat`).
- **Concurrencia:** Uso de semáforos para controlar el acceso concurrente a los metadatos (superbloque, mapa de bits, array de inodos).
- **Capas modulares:** Los módulos (`bloques.c`, `ficheros_basico.c`, `ficheros.c`, `directorios.c`) facilitan el mantenimiento y la evolución del sistema.

## 🖥️ Diagrama de Arquitectura

A continuación se presenta el diagrama de arquitectura del sistema de ficheros ext2. Este diagrama ilustra la relación entre los diferentes módulos y componentes del sistema, así como su interacción con los procesos de usuario.

![Diagrama de Arquitectura](/img/diagrama.png)

### 🔍 Descripción del Diagrama

- **Módulos del Sistema de Ficheros:**  
  Cada módulo (bloques, ficheros, directorios) se encarga de una funcionalidad específica dentro del sistema, lo que permite una fácil extensibilidad y mantenimiento.

- **Interacción del Usuario:**  
  Los comandos de usuario (como `mi_ls`, `mi_cat`, etc.) se comunican con los módulos para ejecutar operaciones sobre el sistema de ficheros.

- **Gestión de Concurrencia:**  
  Los semáforos son utilizados para garantizar la seguridad en el acceso concurrente a los metadatos, evitando condiciones de carrera.

- **Simulación de Acceso a Discos:**  
  El sistema simula un dispositivo de memoria secundaria montado sobre un fichero, lo que permite probar la funcionalidad del sistema sin necesidad de un disco físico.

Este diagrama es fundamental para comprender cómo está estructurado el sistema y cómo los diferentes componentes interactúan entre sí para proporcionar las funcionalidades del sistema de ficheros ext2.

## 📁 Estructura del proyecto

1. **Bibliotecas del sistema de ficheros:**
    - `bloques.c`: Maneja la lectura y escritura de bloques. Esta capa se puede modificar fácilmente para cambiar el medio de almacenamiento.
    - `ficheros_basico.c`: Gestión básica de inodos y bloques.
    - `ficheros.c`: Primitivas para lectura y escritura de ficheros.
    - `directorios.c`: Acceso jerárquico a los directorios.

2. **Programas:**
    - `mi_mkfs`: Crea el sistema de ficheros sobre un fichero (simulando un disco).
    - **Primitivas:** Programas que replican comandos de Unix como `ls`, `cat`, `chmod`, etc.

3. **Simulador de procesos concurrentes:**
    - Simulación de procesos que acceden al sistema de ficheros de manera concurrente, comprobando su consistencia bajo cargas concurrentes.

## 🧪 Pruebas y simulaciones

Se incluyen pruebas detalladas para cada capa del sistema de ficheros, así como una simulación de acceso concurrente en la que **100 procesos** realizan escrituras aleatorias en un fichero, validando el correcto funcionamiento de los punteros directos e indirectos. Las primitivas también pueden ser usadas para verificar el comportamiento del sistema (`mi_cat`, `mi_ls`, etc.).

## 📋 Requisitos del sistema

- **Sistema operativo:** Unix/Linux
- **Arquitectura:** x86_64
- **Compilador:** gcc
- **Dependencias adicionales:** Utilidad `make` para la compilación del proyecto

## 🚀 Instalación y uso

1. Clona el repositorio:
   ```bash
   git clone https://github.com/DavidVazquezRivas/ext2-filesystem.git
   ```

2. Compila el sistema de ficheros y las utilidades:
    ```bash
   make
   ```

3. Crea el sistema de ficheros virtual:
    ```bash
    ./mi_mkfs disco 100000
    ```

4. Utiliza las primitivas para interactuar con el sistema
    ```bash
    ./mi_ls /
    ./mi_cat /archivo.txt
    ./mi_chmod 755 /archivo.txt
    ```

## 📚 Aprendizajes

El desarrollo de este sistema de ficheros ha permitido adquirir y afianzar conocimientos en:

- **Comprensión de los inodos:** Estudio profundo de cómo se organizan los inodos en ext2, y cómo almacenan información tanto sobre los ficheros como sobre su estructura en el sistema.
- **Gestión de enlaces físicos y simbólicos:** Implementación y manejo de enlaces duros, así como su impacto en la estructura de directorios y la gestión de ficheros.
- **Asignación indexada:** Comprensión de la asignación de bloques mediante punteros directos e indirectos (simples, dobles y triples), y cómo esta estructura permite un crecimiento eficiente de los ficheros.
- **Concurrencia y sincronización:** Implementación de mecanismos para evitar condiciones de carrera, especialmente en la gestión de metadatos críticos, utilizando semáforos para proteger el acceso a los recursos compartidos.
- **Lenguaje de programación C:** Profundizando en la gestión de memoria, punteros y estructuras de datos complejas.
- **Diseño modular de sistemas de ficheros:** Entendiendo cómo dividir un sistema complejo en componentes manejables, asegurando separación de responsabilidades entre capas.
- **Simulación de sistemas reales:** Uso de un fichero como dispositivo virtual para almacenar el sistema de ficheros, permitiendo pruebas sin acceso a un disco físico.
- **Herramienta `make`:** Automatización del proceso de compilación y gestión de dependencias con la herramienta `make`.
- **Trabajo en equipo:** Coordinación con varios desarrolladores para diseñar e implementar una solución compleja en un entorno académico, aprendiendo sobre control de versiones y la importancia de la colaboración.

## 🧑‍🤝‍🧑 Autores

Este proyecto ha sido desarrollado como parte de la asignatura **Sistemas Operativos II** en la **Universitat de les Illes Balears (UIB)**.

- **Diseño y arquitectura:** Adelaida Delgado (Profesora de la asignatura)
- **Código:** Pau Antich, [Nasim Benyacoub](https://github.com/nasiiimb), [David Vázquez Rivas](https://github.com/DavidVazquezRivas)
- **Estructura de archivos:** [David Vázquez Rivas](https://github.com/DavidVazquezRivas)