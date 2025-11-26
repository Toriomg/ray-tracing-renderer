#!/bin/bash
#SBATCH --job-name=run-test-jd
#SBATCH --output=./logs/run-test-jd.out

set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

for i in {1..4}
do
echo "[ $i ] Iniciando ejecución $i en $(hostname)"

# Rutas a los archivos de entrada y salida
CONFIG_FILE="res/config_scripts/config${i}example.txt"  
SCENE_FILE="res/scene_scripts/scene${i}example.txt"    
OUTPUT_FILE="out${i}.ppm"

# Ruta a los ejecutables compilados
RENDER_EXE="./out/build/default/par/Release/render-par"
echo ""
echo "========================================="
echo ">>> Midiendo"
echo "========================================="
perf stat -r 5 ${RENDER_EXE} ${SCENE_FILE} ${CONFIG_FILE} ${OUTPUT_FILE}

echo "[ $i ] Mediciones finalizadas."
done
echo "--- Ejecución finalizada ---"