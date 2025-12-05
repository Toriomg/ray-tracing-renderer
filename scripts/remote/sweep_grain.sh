#!/bin/bash
#SBATCH --job-name=grain-sweep
#SBATCH --output=logs/grain_%j.out
#SBATCH --partition=stan
#SBATCH --exclusive
#SBATCH --cpus-per-task=112

export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

EXE="./out/build/default/par/Release/render-par"
SCENE="res/scene_scripts/scene5.txt"
CONFIG="res/config_scripts/config5.txt"
OUTPUT_IMG="out_grain.ppm"
RESULT_FILE="logs/results_grain_sweep.csv"

# Número de hilos óptimo encontrado en el paso anterior (pásalo como argumento)
OPTIMAL_THREADS=${1:-56}

echo "Partitioner,GrainSize,Time(s),Energy(J)" > $RESULT_FILE

echo ">>> PASO 2: EXPLORACIÓN DE GRANULARIDAD (con $OPTIMAL_THREADS hilos fijos) <<<"
echo ">>> Rama analysis/image: Solo procesado de imagen es paralelo <<<"

# Partitioners a probar
PARTITIONERS=("auto" "simple" "static" "affinity")

# Grains para IMAGE: Empezar con grain=threads, luego reducir a la mitad
# PERO también probar granos MÁS GRANDES (típico de tareas memory-bound ligeras)
GRAINS=($OPTIMAL_THREADS)

# Generar reducción a la mitad hasta llegar a 1
CURRENT=$OPTIMAL_THREADS
while [ $CURRENT -gt 1 ]; do
    CURRENT=$((CURRENT / 2))
    GRAINS+=($CURRENT)
done

# Añadir 0 al final (auto de TBB)
GRAINS+=(0)

# Añadir granos grandes típicos de procesado de imagen (1024, 2048, 4096, 8192)
GRAINS+=(1024 2048 4096 8192)

echo "Grains a probar: ${GRAINS[@]}"

for PART in "${PARTITIONERS[@]}"; do
    for GRAIN in "${GRAINS[@]}"; do
        
        # simple_partitioner falla con grain=0, lo saltamos
        if [ "$PART" == "simple" ] && [ "$GRAIN" -eq 0 ]; then continue; fi

        echo "Probando: $PART | Grain: $GRAIN | Threads: $OPTIMAL_THREADS"
        
        # En esta rama: --image-part y --image-grain
        perf stat -e power/energy-pkg/ -o temp.log \
            $EXE $SCENE $CONFIG $OUTPUT_IMG \
            --image-part $PART --image-grain $GRAIN --threads $OPTIMAL_THREADS
        
        TIME=$(grep "seconds time elapsed" temp.log | awk '{print $1}' | tr ',' '.')
        ENERGY=$(grep "Joules" temp.log | awk '{print $1}' | tr ',' '.')
        
        if [ ! -z "$TIME" ]; then
            echo "$PART,$GRAIN,$TIME,$ENERGY" >> $RESULT_FILE
        fi
    done
done

rm temp.log
echo ">>> FIN EXPLORACIÓN DE GRANULARIDAD <<<"
echo ">>> NOTA: Mejoras mínimas esperadas (procesado de imagen es <1% del tiempo total) <<<"
