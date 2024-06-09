/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#!/bin/bash

# Datos del autor
AUTHOR_NAME="Guido Barosio"
AUTHOR_EMAIL="guido@bravo47.com"
DATE=$(date +"%Y-%m-%d")

# Header a agregar
HEADER="/*
 * Autor: $AUTHOR_NAME
 * Email: $AUTHOR_EMAIL
 * Fecha: $DATE
 */

"

# Encontrar y procesar todos los archivos .c de forma recursiva
find . -type f -name "*.c" | while read -r file; do
    # Crear un archivo temporal con el encabezado y el contenido del archivo original
    echo "$HEADER$(cat "$file")" > temp_file && mv temp_file "$file"
    echo "Header agregado a $file"
done
