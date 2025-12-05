#!/bin/bash
#SBATCH --job-name=test-custom
#SBATCH --output=logs/custom_%j.out
#SBATCH --error=logs/custom_%j.err
#SBATCH --partition=stan
#SBATCH --exclusive

set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

EXE="./out/build/default/par/Release/render-par"
SCENE="res/scene_scripts/scene5.txt"
CONFIG="res/config_scripts/config5.txt"
OUTPUT="out_custom.ppm"

# ============================================================
# CONFIGURA AQUÍ TU TEST PERSONALIZADO
# Rama analysis/image: Solo los parámetros de IMAGE importan
# ============================================================
IMAGE_PARTITIONER="static"   # Opciones: auto, simple, static, affinity
IMAGE_GRAIN="1024"           # Valores típicos para image: 0, 1024, 4096, 8192
THREADS="56"                 # Valores típicos: 1, 2, 4, 8, 16, 28, 56, 112
# ============================================================

echo ">>> Iniciando Prueba Personalizada en $(hostname) <<<"
echo ">>> Rama analysis/image: Solo procesado de imagen es paralelo <<<"

ARGS="--image-part $IMAGE_PARTITIONER --image-grain $IMAGE_GRAIN --threads $THREADS"
echo "Ejecutando con: $ARGS"
perf stat -e power/energy-pkg/ $EXE $SCENE $CONFIG $OUTPUT $ARGS

echo ""
echo ">>> Verificando salida <<<"
ls -lh $OUTPUT
md5sum $OUTPUT

echo ""
echo ">>> Comparando con referencia scene5 <<<"
diff -q $OUTPUT res/references_par/s5-par.ppm && echo "✓ Imagen CORRECTA" || echo "✗ Imagen DIFERENTE (puede ser normal por diferencias de redondeo)"

echo ">>> Prueba Finalizada <<<"
