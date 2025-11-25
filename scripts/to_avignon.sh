#!/bin/bash

# --- Configuración ---
ARCHIVE_NAME="render-proyecto.tar.gz"
REMOTE_SERVER="avignon.lab.inf.uc3m.es"
REMOTE_DIR="~/rtx/" # Directorio de destino en Avignon

# 1. Comprobar si se ha pasado un nombre de usuario
if [ -z "$1" ]; then
    echo "Error: No se ha especificado un nombre de usuario."
    echo "Uso: ./upload_to_avignon.sh <tu_usuario_lab>"
    exit 1
fi

USERNAME=$1

# 2. Comprimir el proyecto excluyendo archivos innecesarios
echo ">>> Comprimiendo el proyecto en ${ARCHIVE_NAME}..."
tar --exclude='./build' \
    --exclude='./.git' \
    --exclude='*.ppm' \
    --exclude='*.tar.gz' \
    -czvf "${ARCHIVE_NAME}" .

if [ $? -ne 0 ]; then
    echo "Error: Falló la compresión del proyecto."
    exit 1
fi

echo ">>> Compresión completada."

# 3. Subir el archivo comprimido usando scp
echo ">>> Subiendo ${ARCHIVE_NAME} a ${USERNAME}@${REMOTE_SERVER}:${REMOTE_DIR}"
scp "${ARCHIVE_NAME}" "${USERNAME}@${REMOTE_SERVER}:${REMOTE_DIR}"

if [ $? -eq 0 ]; then
    echo ">>> ¡Subida completada con éxito!"
else
    echo "Error: Falló la subida del archivo."
    exit 1
fi