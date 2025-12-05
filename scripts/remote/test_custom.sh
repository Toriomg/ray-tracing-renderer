#!/bin/bash
# Script para pruebas manuales (Actualizado a Scene 5)

# Busca el ejecutable automáticamente
EXECUTABLE=$(find . -name render-par -type f | head -n 1)
SCENE="res/scene_scripts/scene5.txt"
CONFIG="res/config_scripts/config5.txt"
OUTPUT="output_test.ppm"

if [ -z "$EXECUTABLE" ]; then
    echo "Error: No se encuentra el ejecutable 'render-par'. Compila primero."
    exit 1
fi

echo ">>> Iniciando Prueba Personalizada (Scene 5) <<<"
echo "Binario: $EXECUTABLE"
echo "Argumentos extra: ${@:1}"

# Ejecutar pasando todos los argumentos
$EXECUTABLE "$SCENE" "$CONFIG" "$OUTPUT" "$@"

echo ">>> Fin de la prueba <<<"
