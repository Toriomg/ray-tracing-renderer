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

echo ">>> Iniciando Prueba Personalizada en $(hostname) <<<"

# Solo un renderizado con particionador static
ARGS="--image-part static --image-grain 500 --threads 4"
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
