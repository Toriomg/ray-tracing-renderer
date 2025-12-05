#!/bin/bash
#SBATCH --job-name=test-determinism
#SBATCH --output=logs/determinism_%j.out
#SBATCH --error=logs/determinism_%j.err
#SBATCH --partition=stan
#SBATCH --exclusive

set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

EXE="./out/build/default/par/Release/render-par"
SCENE="res/scene_scripts/scene5.txt"
CONFIG="res/config_scripts/config5.txt"

echo ">>> Test de Determinismo con 1 Thread <<<"

# Test 1: Static con 1 thread
echo "=== Run 1: Static, 1 thread ==="
$EXE $SCENE $CONFIG out_det1.ppm --image-part static --threads 1
md5_1=$(md5sum out_det1.ppm | awk '{print $1}')
echo "MD5: $md5_1"

# Test 2: Auto con 1 thread
echo "=== Run 2: Auto, 1 thread ==="
$EXE $SCENE $CONFIG out_det2.ppm --image-part auto --threads 1
md5_2=$(md5sum out_det2.ppm | awk '{print $1}')
echo "MD5: $md5_2"

# Test 3: Static con 1 thread (repetir)
echo "=== Run 3: Static, 1 thread (repeat) ==="
$EXE $SCENE $CONFIG out_det3.ppm --image-part static --threads 1
md5_3=$(md5sum out_det3.ppm | awk '{print $1}')
echo "MD5: $md5_3"

echo ""
echo "=== Resultados ==="
if [ "$md5_1" == "$md5_2" ] && [ "$md5_1" == "$md5_3" ]; then
  echo "✓ TODAS IDÉNTICAS - Sistema es determinista con 1 thread"
else
  echo "✗ DIFERENTES - Hay non-determinismo incluso con 1 thread"
  echo "  Run1: $md5_1"
  echo "  Run2: $md5_2"
  echo "  Run3: $md5_3"
fi

echo ">>> Finalizado <<<"
