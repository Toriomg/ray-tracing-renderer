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

# Bucle solicitado: de 56 a 112 con salto de 4
# Añadimos también 1, 2, 4, 8... 28 para tener la curva completa desde el principio
for THREADS in 1 2 4 8 16 28 $(seq 56 4 112); do
    
    echo "Probando con $THREADS hilos..."
    
    # IMPORTANTE: Pasamos --threads al programa
    perf stat -e power/energy-pkg/ -o temp.log \
        $EXE $SCENE $CONFIG $OUTPUT_IMG \
        --partitioner $BEST_PART --grain $BEST_GRAIN --threads $THREADS
    
    TIME=$(grep "seconds time elapsed" temp.log | awk '{print $1}' | tr ',' '.')
    ENERGY=$(grep "Joules" temp.log | awk '{print $1}' | tr ',' '.')
    
    if [ ! -z "$TIME" ]; then
        echo "$THREADS,$TIME,$ENERGY" >> $RESULT_FILE
    fi
done

rm temp.log
echo ">>> FIN ESCALABILIDAD <<<"