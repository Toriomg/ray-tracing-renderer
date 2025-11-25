#!/bin/bash
source env.sh
echo ">>> Sincronizando con Avignon..."

# --delete borra archivos en el servidor que ya no existen en tu local (limpieza automática)
rsync -avz --delete \
    --exclude '.git' \
    --exclude 'out/build' \
    --exclude '__pycache__' \
    --exclude 'out' \
    --exclude '*.ppm' \
    --exclude '.vscode' \
    ./ $REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR

echo ">>> Sincronización completada."