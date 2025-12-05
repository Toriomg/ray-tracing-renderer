#!/bin/bash
#SBATCH --job-name=comprehensive-test
#SBATCH --output=logs/comprehensive_%j.out
#SBATCH --error=logs/comprehensive_%j.err
#SBATCH --partition=stan
#SBATCH --exclusive

set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

EXE="./out/build/default/par/Release/render-par"
SCENE="res/scene_scripts/scene5.txt"
CONFIG="res/config_scripts/config5.txt"
RESULT_FILE="logs/comprehensive_results.csv"

echo ">>> Test Comprehensivo de PPM Writer - Rama analysis/writer <<<"
echo ""

# Header CSV
echo "Test,Partitioner,GrainSize,Threads,Time(s),Energy(J)" > $RESULT_FILE

run_test() {
  local test_name=$1
  local part=$2
  local grain=$3
  local threads=$4
  local output="out_comp_${test_name}.ppm"
  
  echo "=== Test: $test_name | Part: $part | Grain: $grain | Threads: $threads ==="
  
  # Ejecutar con perf
  perf stat -e power/energy-pkg/ -o temp_perf.log \
    $EXE $SCENE $CONFIG $output \
    --image-part $part --image-grain $grain --threads $threads 2>&1
  
  # Extraer tiempo y energía
  local time=$(grep "elapsed" temp_perf.log | awk '{print $1}')
  local energy=$(grep "Joules" temp_perf.log | awk '{print $1}')
  
  echo "$test_name,$part,$grain,$threads,$time,$energy" >> $RESULT_FILE
  
  # MD5 para verificar
  local md5=$(md5sum $output | awk '{print $1}')
  echo "  Time: $time | Energy: $energy | MD5: $md5"
  echo ""
}

echo ">>> FASE 1: Variación de Particionadores (grain=500, threads=4) <<<"
run_test "part_auto" "auto" 0 4
run_test "part_simple" "simple" 500 4
run_test "part_static" "static" 500 4
run_test "part_affinity" "affinity" 500 4

echo ">>> FASE 2: Variación de Grain Size (static, threads=4) <<<"
run_test "grain_100" "static" 100 4
run_test "grain_500" "static" 500 4
run_test "grain_1000" "static" 1000 4
run_test "grain_5000" "static" 5000 4

echo ">>> FASE 3: Escalabilidad de Threads (static, grain=500) <<<"
run_test "threads_1" "static" 500 1
run_test "threads_2" "static" 500 2
run_test "threads_4" "static" 500 4
run_test "threads_8" "static" 500 8
run_test "threads_16" "static" 500 16

echo ">>> FASE 4: Combinaciones Extremas <<<"
run_test "extreme_tiny" "simple" 10 8
run_test "extreme_large" "affinity" 10000 2
run_test "extreme_threads" "auto" 0 28

echo ""
echo ">>> Resultados guardados en $RESULT_FILE <<<"
echo ">>> Para descargar: scp user@avignon:rtx/$RESULT_FILE ./ <<<"
echo ">>> Test Finalizado <<<"
