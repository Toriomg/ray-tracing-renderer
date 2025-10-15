#!/bin/bash

set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

echo "--- Iniciando ejecución en $(hostname) ---"

# Rutas a los archivos de entrada y salida
CONFIG_FILE="res/config_simple.txt"  # Cambia por tu archivo de config
SCENE_FILE="res/scene_simple.txt"    # Cambia por tu archivo de escena
OUTPUT_FILE_SOA="outputImageSOA.ppm"
OUTPUT_FILE_AOS="outputImageAOS.ppm"

# Ruta a los ejecutables compilados
RENDER_SOA_EXE="./out/build/default/soa/Release/render-soa"
RENDER_AOS_EXE="./out/build/default/aos/Release/render-aos"
echo ""
echo "========================================="
echo ">>> Midiendo 'render-soa' (5 ejecuciones)"
echo "========================================="
perf stat -r 5 ${RENDER_SOA_EXE} #${CONFIG_FILE} ${SCENE_FILE} ${OUTPUT_FILE_SOA}

# --- Medición de rendimiento para render-aos ---
echo ""
echo "========================================="
echo ">>> Midiendo 'render-aos' (5 ejecuciones)"
echo "========================================="
#perf stat -r 5 ${RENDER_AOS_EXE} #${CONFIG_FILE} ${SCENE_FILE} ${OUTPUT_FILE_AOS}
echo "main aos por hacer bien aun"

echo "--- Mediciones finalizadas. ---"
echo "--- Ejecución finalizada ---"