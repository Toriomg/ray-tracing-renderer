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
SCENE="res/scene_scripts/scene5.txt"   # Escena pesada para pruebas notables
CONFIG="res/config_scripts/config5.txt"
OUTPUT="out_custom.ppm"

echo ">>> Iniciando Prueba Personalizada en $(hostname) <<<"

# Búsqueda robusta del ejecutable
EXE=$(find . -name render-par -type f | head -n 1)

if [[ -z "$EXE" ]]; then
  echo "ERROR: No se encontró el ejecutable render-par"
  echo "Por favor, compila el proyecto primero con: cmake --build out/build/default"
  exit 1
fi

if [[ ! -x "$EXE" ]]; then
  echo "ERROR: El archivo $EXE existe pero no es ejecutable"
  exit 1
fi

echo "Ejecutable encontrado: $EXE"
echo "Argumentos recibidos: $@"

# Ejecutamos midiendo tiempo con time (sin perf)
echo "Iniciando renderizado..."
time "$EXE" "$SCENE" "$CONFIG" "$OUTPUT" "$@"

echo ">>> Prueba Finalizada <<<"
echo "Salida generada en: $OUTPUT"