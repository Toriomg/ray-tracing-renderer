#!/bin/bash

set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

echo "--- Iniciando ejecución 1 en $(hostname) ---"

# Rutas a los archivos de entrada y salida
CONFIG_FILE="res/config_scripts/config1example.txt"  
SCENE_FILE="res/scene_scripts/scene1example.txt"    
OUTPUT_FILE="out1.ppm"

# Ruta a los ejecutables compilados
RENDER_EXE="./out/build/default/par/Release/render-par"
echo ""
echo "========================================="
echo ">>> Midiendo 'render-soa' (5 ejecuciones)"
echo "========================================="
perf stat -r 5 ${RENDER_EXE} ${SCENE_FILE} ${CONFIG_FILE} ${OUTPUT_FILE}

echo "--- Mediciones finalizadas. ---"
echo "--- Ejecución finalizada ---"