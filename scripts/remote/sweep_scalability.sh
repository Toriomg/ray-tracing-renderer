#!/bin/bash
#SBATCH --job-name=scale-sweep
#SBATCH --output=logs/scale_%j.out
#SBATCH --partition=stan
#SBATCH --exclusive
#SBATCH --cpus-per-task=112

export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

EXE="./out/build/default/par/Release/render-par"
SCENE="res/scene_scripts/scene5.txt"
CONFIG="res/config_scripts/config5.txt"
OUTPUT_IMG="out_scale.ppm"
RESULT_FILE="logs/results_scalability.csv"

# --- CONFIGURACIÓN ÓPTIMA FIJA (Actualiza esto tras el primer test) ---
BEST_PART=${1:-static}   # Usa el primer argumento, o 'static' por defecto
BEST_GRAIN=${2:-64}      # Usa el segundo argumento, o '64' por defecto
# --- RANGO DE HILOS PERSONALIZABLE ---
THREAD_START=${3:-}      # Inicio del rango (vacío = modo híbrido)
THREAD_END=${4:-120}     # Fin del rango (por defecto 120)
THREAD_STEP=${5:-4}      # Paso del rango (por defecto 4)
# ----------------------------------------------------------------------

# Crear cabecera solo si el archivo no existe (para permitir ejecuciones incrementales)
if [ ! -f "$RESULT_FILE" ]; then
    echo "Threads,Time(s),Energy(J)" > $RESULT_FILE
    sync
fi

# Determinar qué secuencia de hilos usar
if [ -z "$THREAD_START" ]; then
    # Modo híbrido (por defecto): valores bajos + barrido fino
    THREAD_SEQUENCE="1 2 4 8 16 28 $(seq 56 $THREAD_STEP $THREAD_END)"
    echo ">>> INICIANDO TEST DE ESCALABILIDAD ($BEST_PART / $BEST_GRAIN) <<<"
    echo ">>> Modo híbrido: 1,2,4,8,16,28 + seq(56,$THREAD_STEP,$THREAD_END) <<<"
else
    # Modo personalizado: rango especificado
    THREAD_SEQUENCE=$(seq $THREAD_START $THREAD_STEP $THREAD_END)
    echo ">>> INICIANDO TEST DE ESCALABILIDAD ($BEST_PART / $BEST_GRAIN) <<<"
    echo ">>> Rango personalizado: $THREAD_START → $THREAD_END (paso $THREAD_STEP) <<<"
fi

for THREADS in $THREAD_SEQUENCE; do
    
    echo "Probando con $THREADS hilos..."
    
    # Ejecutar perf y capturar el exit code sin abortar el script
    set +e
    perf stat -r 5 -e power/energy-pkg/ -o temp.log \
        $EXE $SCENE $CONFIG $OUTPUT_IMG \
        --image-part $BEST_PART --image-grain $BEST_GRAIN --threads $THREADS 2>&1
    PERF_EXIT=$?
    set -e
    
    # Si perf falló, registrar error y continuar
    if [ $PERF_EXIT -ne 0 ]; then
        echo "ERROR: perf stat falló para threads=$THREADS (exit code: $PERF_EXIT)" >&2
        echo "$THREADS,ERROR,ERROR" >> $RESULT_FILE
        sync
        continue
    fi
    
    # Parsear con protección (|| true evita que grep devuelva error fatal)
    TIME=$(grep "seconds time elapsed" temp.log 2>/dev/null | awk '{print $1}' | tr ',' '.' || echo "N/A")
    ENERGY=$(grep "Joules" temp.log 2>/dev/null | awk '{print $1}' | tr ',' '.' || echo "N/A")
    
    # Validar que se extrajeron ambos valores
    if [ "$TIME" = "N/A" ] || [ "$ENERGY" = "N/A" ] || [ -z "$TIME" ] || [ -z "$ENERGY" ]; then
        echo "ERROR: No se pudo extraer TIME o ENERGY para threads=$THREADS" >&2
        echo "$THREADS,PARSE_ERROR,PARSE_ERROR" >> $RESULT_FILE
        sync
        continue
    fi
    
    # Escribir resultado válido y forzar sync a disco
    echo "$THREADS,$TIME,$ENERGY" >> $RESULT_FILE
    sync
done

rm -f temp.log
echo ">>> FIN ESCALABILIDAD <<<"