#!/bin/bash
#SBATCH --job-name=test-custom
#SBATCH --output=logs/custom_%j.out
#SBATCH --error=logs/custom_%j.err
#SBATCH --partition=stan
#SBATCH --exclusive

# --- ESTO ES LO QUE HACE QUE FUNCIONE (Cargamos el entorno correcto del nodo Obrero) ---
set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# Definiciones
EXE="./out/build/default/par/Release/render-par"
SCENE="res/scene_scripts/scene5.txt"   # Cambia a scene2.txt si quieres ir rápido
CONFIG="res/config_scripts/config5.txt"
OUTPUT="out_custom.ppm"

echo ">>> Iniciando Prueba Personalizada en $(hostname) <<<"

# --- Edita esta línea para probar lo que quieras ---
# Aquí ponemos la combinación que intentaste hacer a mano y falló
ARGS="--render-part static --render-grain 32 --image-part auto --image-grain 0 --threads 4"

echo "Ejecutando con: $ARGS"

# Ejecutamos midiendo tiempo y energía
perf stat -e power/energy-pkg/ $EXE $SCENE $CONFIG $OUTPUT $ARGS

echo ">>> Prueba Finalizada <<<"