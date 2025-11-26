#!/bin/bash

# 1. Recibimos las variables del Makefile
USER="$1"
HOST="$2"
DIR="$3"

PASSFILE=".password"

# Si alguna variable está vacía, paramos para evitar el error "usage: ssh..."
if [ -z "$USER" ] || [ -z "$HOST" ]; then
    echo "ERROR CRÍTICO: Variables no recibidas."
    echo "Uso correcto desde Makefile: bash sync.sh <USER> <HOST> <DIR>"
    exit 1
fi
if [ ! -f "$PASSFILE" ]; then
    echo "ERROR: No encuentro el archivo .password en la raíz."
    exit 1
fi

echo ">>> Conectando como $USER a $HOST..."

# 2. Crear carpeta remota
sshpass -f "$PASSFILE" ssh -o StrictHostKeyChecking=no "${USER}@${HOST}" "mkdir -p ${DIR}"

# 3. Subir archivos
echo ">>> Subiendo archivos..."
tar --exclude='./out' \
    --exclude='./.git' \
    --exclude='./.vscode' \
    --exclude='./__pycache__' \
    --exclude='*.tar.gz' \
    -czf - . | sshpass -f "$PASSFILE" ssh -o StrictHostKeyChecking=no "${USER}@${HOST}" "tar -xzf - -C ${DIR}"

echo ">>> ¡Subida completada!"