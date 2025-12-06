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
BEST_PART=${1:-static}  # Usa el primer argumento, o 'static' por defecto
BEST_GRAIN=${2:-64}     # Usa el segundo argumento, o '64' por defecto
# ----------------------------------------------------------------------

echo "Threads,Time(s),Energy(J)" > $RESULT_FILE

echo ">>> INICIANDO TEST DE ESCALABILIDAD ($BEST_PART / $BEST_GRAIN) <<<"

# Bucle con valores iniciales + barrido fino de 56 a 120 (de 4 en 4)
# Valores clave: 28 (1 socket), 56 (2 sockets físicos), 112 (máx. físico), 120 (sobrecarga)
for THREADS in 1 2 4 8 16 28 $(seq 56 4 120); do
    
    echo "Probando con $THREADS hilos..."
    
    # IMPORTANTE: En analysis/writer usamos --writer-part y --writer-grain
    perf stat -r 5 -e power/energy-pkg/ -o temp.log \
        $EXE $SCENE $CONFIG $OUTPUT_IMG \
        --writer-part $BEST_PART --writer-grain $BEST_GRAIN --threads $THREADS 2>&1
    
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