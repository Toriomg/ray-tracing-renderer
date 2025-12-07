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

echo "Threads,Time(s),Energy(J)" > $RESULT_FILE

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
    
    # En esta rama (rendering): --render-part y --render-grain
    perf stat -r 5 -e power/energy-pkg/ -o temp.log \
        $EXE $SCENE $CONFIG $OUTPUT_IMG \
        --render-part $BEST_PART --render-grain $BEST_GRAIN --threads $THREADS 2>&1
    
    # Verificar que perf se ejecutó correctamente
    if [ $? -ne 0 ]; then
        echo "ERROR: perf stat falló para threads=$THREADS" >&2
        continue
    fi
    
    TIME=$(grep "seconds time elapsed" temp.log | awk '{print $1}' | tr ',' '.')
    ENERGY=$(grep "Joules" temp.log | awk '{print $1}' | tr ',' '.')
    
    # Validar que se extrajeron ambos valores
    if [ -z "$TIME" ] || [ -z "$ENERGY" ]; then
        echo "ERROR: No se pudo extraer TIME o ENERGY para threads=$THREADS" >&2
        continue
    fi
    
    # Escribir resultado y forzar sync a disco
    echo "$THREADS,$TIME,$ENERGY" >> $RESULT_FILE
    sync
done

rm -f temp.log
echo ">>> FIN ESCALABILIDAD <<<"