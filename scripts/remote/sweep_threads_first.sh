#!/bin/bash
#SBATCH --job-name=threads-first
#SBATCH --output=logs/threads_first_%j.out
#SBATCH --partition=stan
#SBATCH --exclusive
#SBATCH --cpus-per-task=112

export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

EXE="./out/build/default/par/Release/render-par"
SCENE="res/scene_scripts/scene5.txt"
CONFIG="res/config_scripts/config5.txt"
OUTPUT_IMG="out_threads.ppm"
RESULT_FILE="logs/results_threads_first.csv"

echo "Threads,Time(s),Energy(J)" > $RESULT_FILE

echo ">>> PASO 1: EXPLORACIÓN INICIAL DE HILOS (con auto_partitioner) <<<"
echo ">>> Rama analysis/image: Solo procesado de imagen es paralelo <<<"

# Valores clave según el profesor:
# 28 hilos: llena un socket completo 
# 56 hilos: máximo paralelismo físico pero latencia entre CPUs
# 112 hilos: máximo posible
# 120 hilos: sobrecarga para verificar
for THREADS in 28 56 112 120; do
    
    echo "Probando con $THREADS hilos (auto_partitioner, grain=0)..."
    
    # Usamos auto_partitioner (TBB decide el grain óptimo)
    # En esta rama: --image-part y --image-grain
    perf stat -e power/energy-pkg/ -o temp.log \
        $EXE $SCENE $CONFIG $OUTPUT_IMG \
        --image-part auto --image-grain 0 --threads $THREADS
    
    TIME=$(grep "seconds time elapsed" temp.log | awk '{print $1}' | tr ',' '.')
    ENERGY=$(grep "Joules" temp.log | awk '{print $1}' | tr ',' '.')
    
    if [ ! -z "$TIME" ]; then
        echo "$THREADS,$TIME,$ENERGY" >> $RESULT_FILE
    fi
done

rm temp.log
echo ">>> FIN EXPLORACIÓN INICIAL DE HILOS <<<"
echo ">>> NOTA: En esta rama, la mejora será mínima (procesado de imagen < 1% del tiempo total) <<<"
echo ">>> Analiza los resultados para decidir el número óptimo de hilos <<<"
echo ">>> Luego ejecuta: make sweep-grain THREADS=<óptimo> <<<"
