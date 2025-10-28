#!/bin/bash

set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

echo "--- Iniciando ejecución 2 en $(hostname) ---"

# Rutas a los archivos de entrada y salida
CONFIG_FILE="res/config_scripts/config2example.txt"  
SCENE_FILE="res/scene_scripts/scene2example.txt"  
OUTPUT_FILE_SOA="out2_SOA.ppm"
OUTPUT_FILE_AOS="out2_AOS.ppm"

# Ruta a los ejecutables compilados
RENDER_SOA_EXE="./out/build/default/soa/Release/render-soa"
RENDER_AOS_EXE="./out/build/default/aos/Release/render-aos"
echo ""
echo "========================================="
echo ">>> Midiendo 'render-soa' (5 ejecuciones)"
echo "========================================="
perf stat -r 5 ${RENDER_SOA_EXE} ${SCENE_FILE} ${CONFIG_FILE} ${OUTPUT_FILE_SOA}

# --- Medición de rendimiento para render-aos ---
echo ""
echo "========================================="
echo ">>> Midiendo 'render-aos' (5 ejecuciones)"
echo "========================================="
perf stat -r 5 ${RENDER_AOS_EXE} ${SCENE_FILE} ${CONFIG_FILE} ${OUTPUT_FILE_AOS}

echo "--- Mediciones finalizadas. ---"
echo "--- Ejecución finalizada ---"