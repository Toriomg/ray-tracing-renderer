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
echo ">>> Rama analysis/rendering: Solo renderizado es paralelo <<<"

# Partitioners a probar
PARTITIONERS=("auto" "simple" "static" "affinity")

# Grains para RENDERING: Empezar con grain=threads, luego reducir a la mitad
GRAINS=($OPTIMAL_THREADS)

# Generar reducción a la mitad hasta llegar a 1
CURRENT=$OPTIMAL_THREADS
while [ $CURRENT -gt 1 ]; do
    CURRENT=$((CURRENT / 2))
    GRAINS+=($CURRENT)
done

# Añadir 0 al final (auto de TBB)
GRAINS+=(0)

# Añadir granos típicos de rendering (32, 64, 128, 256, 512, 1024)
GRAINS+=(32 64 128 256 512 1024)

echo "Grains a probar: ${GRAINS[@]}"

for PART in "${PARTITIONERS[@]}"; do
    for GRAIN in "${GRAINS[@]}"; do
        
        # simple_partitioner falla con grain=0, lo saltamos
        if [ "$PART" == "simple" ] && [ "$GRAIN" -eq 0 ]; then continue; fi

        echo "Probando: $PART | Grain: $GRAIN | Threads: $OPTIMAL_THREADS"
        
        # En esta rama: --render-part y --render-grain
        perf stat -r 5 -e power/energy-pkg/ -o temp.log \
            $EXE $SCENE $CONFIG $OUTPUT_IMG \
            --render-part $PART --render-grain $GRAIN --threads $OPTIMAL_THREADS 2>&1
        
        # Verificar que perf se ejecutó correctamente
        if [ $? -ne 0 ]; then
            echo "ERROR: perf stat falló para $PART / grain=$GRAIN / threads=$OPTIMAL_THREADS" >&2
            continue
        fi
        
        TIME=$(grep "seconds time elapsed" temp.log | awk '{print $1}' | tr ',' '.')
        ENERGY=$(grep "Joules" temp.log | awk '{print $1}' | tr ',' '.')
        
        # Validar que se extrajeron ambos valores
        if [ -z "$TIME" ] || [ -z "$ENERGY" ]; then
            echo "ERROR: No se pudo extraer TIME o ENERGY para $PART / grain=$GRAIN" >&2
            continue
        fi
        
        # Escribir resultado y forzar sync a disco
        echo "$PART,$GRAIN,$TIME,$ENERGY" >> $RESULT_FILE
        sync
    done
done

rm -f temp.log
echo ">>> FIN EXPLORACIÓN DE GRANULARIDAD <<<"
