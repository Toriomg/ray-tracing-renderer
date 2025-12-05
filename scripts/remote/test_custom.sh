#!/bin/bash
#SBATCH --job-name=test-custom
#SBATCH --output=logs/custom_%j.out
#SBATCH --error=logs/custom_%j.err
#SBATCH --partition=stan
#SBATCH --exclusive

# --- LA LÍNEA MÁGICA QUE FALTABA (Sin esto, falla) ---
set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# Definiciones
EXE="./out/build/default/par/Release/render-par"
SCENE="res/scene_scripts/scene5.txt"
CONFIG="res/config_scripts/config5.txt"
OUTPUT="out_custom.ppm"

echo ">>> Iniciando Prueba Personalizada (Fix Libs) en $(hostname) <<<"

# --- CONFIGURACIÓN MANUAL PARA RAMA IMAGEN ---
# Motor Secuencial (ignore grain) + Imagen Paralela (auto) + 4 Hilos
ARGS="--render-part auto --render-grain 0 --image-part static --image-grain 1024 --threads 4"

echo "Ejecutando con: $ARGS"

# Ejecutamos midiendo tiempo
# Usamos 'time' de bash además de perf por si perf falla
time $EXE $SCENE $CONFIG $OUTPUT $ARGS

echo ">>> Prueba Finalizada <<<"