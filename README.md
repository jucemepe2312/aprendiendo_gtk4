##Aprendiendo GTK4

Repositorio de práctica y aprendizaje de GTK4 en lenguaje C. Contiene ejemplos progresivos que van desde una ventana básica hasta el uso de botones y contenedores.
📋 Contenido
Archivo	Descripción
hola.c	Programa que muestra un botón; al presionarlo imprime "Hello World" en consola.
botones.c	Ejemplo con botones.
gtkbox.c	Inicia una ventana con un GtkBox como contenedor.
compile_commands.json	Archivo de configuración para herramientas de compilación (clangd, etc.).
.gitignore	Archivos ignorados por Git.

También se incluyen los binarios compilados (hola, botones) como referencia.
🛠️ Requisitos

    GTK4 instalado en el sistema.

    GCC o Clang.

    pkg-config para obtener las flags de compilación.

Instalación de GTK4

Debian/Ubuntu:
bash

sudo apt install libgtk-4-dev

Fedora:
bash

sudo dnf install gtk4-devel

Arch Linux:
bash

sudo pacman -S gtk4

🚀 Compilación

Para compilar cualquiera de los ejemplos, usa pkg-config para obtener las flags necesarias:
bash

gcc hola.c -o hola $(pkg-config --cflags --libs gtk4)

Para gtkbox.c:
bash

gcc gtkbox.c -o gtkbox $(pkg-config --cflags --libs gtk4)

##▶️ Ejecución
bash

./hola

Al ejecutarse, se abrirá una ventana con un botón. Al presionarlo, se imprimirá "Hello World" en la terminal.
📁 Estructura del proyecto
text

aprendiendo_gtk4/
├── .gitignore
├── botones
├── botones.c
├── compile_commands.json
├── gtkbox.c
├── hola
└── hola.c

📝 Notas

    Este repositorio es parte de un proceso de aprendizaje, por lo que el código puede evolucionar con el tiempo.

    Los archivos binarios (hola, botones) se incluyen solo como referencia; se recomienda recompilar desde el código fuente.

📄 Licencia

Este proyecto no especifica una licencia. Si deseas usarlo, contacta al autor.
