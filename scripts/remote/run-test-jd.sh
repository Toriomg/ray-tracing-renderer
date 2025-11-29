#!/bin/bash
#SBATCH --job-name=run-test-jd-${TIMESTAMP}
#SBATCH --output=./logs/run-test-jd.out

set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# Configuración SOLO para el Escenario 5
i=5
echo "[ $i ] Iniciando ejecución CRÍTICA Escenario $i en $(hostname)"

CONFIG_FILE="res/config_scripts/config${i}.txt"  
SCENE_FILE="res/scene_scripts/scene${i}.txt"    
OUTPUT_FILE="out${i}_par.ppm"

# Ejecutable
RENDER_EXE="./out/build/default/par/Release/render-par"

echo ">>> Ejecutando Escenario $i (Objetivo < 175s)..."

# Usamos perf para medir tiempo y energía
# -r 1: Solo una ejecución para no bloquear la cola (si quieres más precisión, pon 3 o 5)
perf stat -r 1 -e power/energy-pkg/ ${RENDER_EXE} ${SCENE_FILE} ${CONFIG_FILE} ${OUTPUT_FILE}

echo "--------------------------------------------------"
echo "--- Prueba S5 Finalizada ---"