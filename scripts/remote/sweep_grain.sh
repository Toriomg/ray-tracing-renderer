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

# --- ARGUMENTOS PERSONALIZABLES (para ejecución por lotes) ---
OPTIMAL_THREADS=${1:-56}  # Número de hilos (por defecto 56)
CUSTOM_PARTS="$2"         # Lista de partitioners (ej: "static affinity")
CUSTOM_GRAINS="$3"        # Lista de grains (ej: "32 64 128")
# ---------------------------------------------------------------

# Crear cabecera solo si el archivo no existe (para permitir ejecuciones incrementales)
if [ ! -f "$RESULT_FILE" ]; then
    echo "Partitioner,GrainSize,Time(s),Energy(J)" > $RESULT_FILE
    sync
fi

echo ">>> PASO 2: EXPLORACIÓN DE GRANULARIDAD (con $OPTIMAL_THREADS hilos fijos) <<<"
echo ">>> Rama analysis/image: Solo procesado de imagen es paralelo <<<"

# Partitioners a probar (usar custom si se proporciona, sino usar defaults)
if [ -n "$CUSTOM_PARTS" ]; then
    PARTITIONERS=($CUSTOM_PARTS)
    echo ">>> Partitioners personalizados: ${PARTITIONERS[@]} <<<"
else
    PARTITIONERS=("auto" "simple" "static" "affinity")
    echo ">>> Partitioners por defecto: ${PARTITIONERS[@]} <<<"
fi

# Grains a probar (usar custom si se proporciona, sino generar automáticamente)
if [ -n "$CUSTOM_GRAINS" ]; then
    GRAINS=($CUSTOM_GRAINS)
    echo ">>> Grains personalizados: ${GRAINS[@]} <<<"
else
    # Lógica automática: Empezar con grain=threads, luego reducir a la mitad
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
    
    echo ">>> Grains generados automáticamente: ${GRAINS[@]} <<<"
fi

for PART in "${PARTITIONERS[@]}"; do
    for GRAIN in "${GRAINS[@]}"; do
        
        # simple_partitioner falla con grain=0, lo saltamos
        if [ "$PART" == "simple" ] && [ "$GRAIN" -eq 0 ]; then continue; fi

        echo "Probando: $PART | Grain: $GRAIN | Threads: $OPTIMAL_THREADS"
        
        # Ejecutar perf y capturar exit code sin abortar
        set +e
        perf stat -r 5 -e power/energy-pkg/ -o temp.log \
            $EXE $SCENE $CONFIG $OUTPUT_IMG \
            --image-part $PART --image-grain $GRAIN --threads $OPTIMAL_THREADS 2>&1
        PERF_EXIT=$?
        set -e
        
        # Si perf falló, registrar error y continuar
        if [ $PERF_EXIT -ne 0 ]; then
            echo "ERROR: perf stat falló para $PART / grain=$GRAIN (exit: $PERF_EXIT)" >&2
            echo "$PART,$GRAIN,ERROR,ERROR" >> $RESULT_FILE
            sync
            continue
        fi
        
        # Parsear con protección
        TIME=$(grep "seconds time elapsed" temp.log 2>/dev/null | awk '{print $1}' | tr ',' '.' || echo "N/A")
        ENERGY=$(grep "Joules" temp.log 2>/dev/null | awk '{print $1}' | tr ',' '.' || echo "N/A")
        
        # Validar extracción
        if [ "$TIME" = "N/A" ] || [ "$ENERGY" = "N/A" ] || [ -z "$TIME" ] || [ -z "$ENERGY" ]; then
            echo "ERROR: No se pudo extraer TIME o ENERGY para $PART / grain=$GRAIN" >&2
            echo "$PART,$GRAIN,PARSE_ERROR,PARSE_ERROR" >> $RESULT_FILE
            sync
            continue
        fi
        
        # Escribir resultado válido y forzar sync a disco
        echo "$PART,$GRAIN,$TIME,$ENERGY" >> $RESULT_FILE
        sync
    done
done

rm -f temp.log
echo ">>> FIN EXPLORACIÓN DE GRANULARIDAD <<<"
