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
# ============================================================
PARTITIONER="static"      # Opciones: auto, simple, static, affinity
GRAIN_SIZE="64"           # Valores típicos: 0, 1, 32, 64, 128, 256, 512, 1024
THREADS="56"              # Valores típicos: 1, 2, 4, 8, 16, 28, 56, 112
# ============================================================

echo ">>> Iniciando Prueba Personalizada en $(hostname) <<<"

ARGS="--render-part $PARTITIONER --render-grain $GRAIN_SIZE --threads $THREADS"
echo "Ejecutando con: $ARGS"
perf stat -e power/energy-pkg/ $EXE $SCENE $CONFIG $OUTPUT $ARGS

echo ""
echo ">>> Verificando salida <<<

"
ls -lh $OUTPUT
md5sum $OUTPUT

echo ""
echo ">>> Comparando con referencia scene5 <<<"
diff -q $OUTPUT res/references_par/s5-par.ppm && echo "✓ Imagen CORRECTA" || echo "✗ Imagen DIFERENTE (esperado: render secuencial vs paralelo)"

echo ">>> Prueba Finalizada <<<"
