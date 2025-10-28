#!/bin/bash
# Script de prueba autogenerado para: matte_cilindro_1_obj

set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

echo "--- Iniciando prueba 'matte_cilindro_1_obj' en $(hostname) ---"

# Rutas a los archivos de entrada y salida
CONFIG_FILE="tests_de_escenas/config_vacio.txt"
SCENE_FILE="tests_de_escenas/02_matte_cilindro/scene_1_obj.txt"
OUTPUT_FILE_SOA="tests_de_escenas/1_obj_SOA.ppm"
OUTPUT_FILE_AOS="tests_de_escenas/1_obj_AOS.ppm"

# Ruta a los ejecutables
RENDER_SOA_EXE="/workspace/scripts/testRen/../../out/build/default/soa/Release/render-soa"
RENDER_AOS_EXE="/workspace/scripts/testRen/../../out/build/default/aos/Release/render-aos"

# Comando de medición con energía
PERF_COMMAND="perf stat -r 3 -e cycles,instructions,power/energy-pkg/"

# --- Medición de rendimiento para render-soa ---
echo ""
echo "========================================="
echo ">>> Midiendo 'render-soa' (3 ejecuciones)"
echo "========================================="
#$PERF_COMMAND ${RENDER_SOA_EXE} ${SCENE_FILE} ${CONFIG_FILE} ${OUTPUT_FILE_SOA}
 ${RENDER_SOA_EXE} ${SCENE_FILE} ${CONFIG_FILE} ${OUTPUT_FILE_SOA}

# --- Medición de rendimiento para render-aos ---
echo ""
echo "========================================="
echo ">>> Midiendo 'render-aos' (3 ejecuciones)"
echo "========================================="
#$PERF_COMMAND ${RENDER_AOS_EXE} ${SCENE_FILE} ${CONFIG_FILE} ${OUTPUT_FILE_AOS}
 ${RENDER_AOS_EXE} ${SCENE_FILE} ${CONFIG_FILE} ${OUTPUT_FILE_AOS}

echo "--- Prueba 'matte_cilindro_1_obj' finalizada. ---"
