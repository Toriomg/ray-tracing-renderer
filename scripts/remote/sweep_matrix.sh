#!/bin/bash
#SBATCH --job-name=matrix-sweep
#SBATCH --output=logs/matrix_%j.out
#SBATCH --partition=stan
#SBATCH --exclusive
#SBATCH --cpus-per-task=112

export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
EXE="./out/build/default/par/Release/render-par"
SCENE="res/scene_scripts/scene5.txt"
CONFIG="res/config_scripts/config5.txt"
OUTPUT_IMG="out_matrix.ppm"
RESULT_FILE="logs/results_matrix.csv"

echo "RenderPart,RenderGrain,ImagePart,ImageGrain,Threads,Time(s),Energy(J)" > $RESULT_FILE

# --- ZONA EDITABLE PARA TUS COMPAÑEROS ---

# 1. RANGOS PARA EL MOTOR (Lo más importante)
RENDER_PARTS=("static" "affinity")
RENDER_GRAINS=(32 64)

# 2. RANGOS PARA LA IMAGEN (Segmento individual)
# Aquí podéis probar si cambiar esto afecta en algo (spoiler: poco, pero cubrís el 100%)
IMAGE_PARTS=("auto" "static")
IMAGE_GRAINS=(0)

# 3. RANGOS DE HILOS (Manual y específico)
# Podéis poner la lista exacta que queráis probar
THREAD_LIST=(1 28 56 64 112)
# O generar una secuencia: $(seq 56 4 80)

# -----------------------------------------

echo ">>> INICIANDO BARRIDO MATRICIAL (Combinación Total) <<<"

for R_PART in "${RENDER_PARTS[@]}"; do
  for R_GRAIN in "${RENDER_GRAINS[@]}"; do
    for I_PART in "${IMAGE_PARTS[@]}"; do
      for I_GRAIN in "${IMAGE_GRAINS[@]}"; do
        for THREADS in "${THREAD_LIST[@]}"; do
            
            echo "Testing: Render[$R_PART/$R_GRAIN] Image[$I_PART/$I_GRAIN] Threads[$THREADS]"
            
            # Ejecución con TODOS los flags nuevos
            perf stat -e power/energy-pkg/ -o temp.log \
                $EXE $SCENE $CONFIG $OUTPUT_IMG \
                --render-part $R_PART --render-grain $R_GRAIN \
                --image-part $I_PART --image-grain $I_GRAIN \
                --threads $THREADS

            TIME=$(grep "seconds time elapsed" temp.log | awk '{print $1}' | tr ',' '.')
            ENERGY=$(grep "Joules" temp.log | awk '{print $1}' | tr ',' '.')

            if [ ! -z "$TIME" ]; then
                echo "$R_PART,$R_GRAIN,$I_PART,$I_GRAIN,$THREADS,$TIME,$ENERGY" >> $RESULT_FILE
            fi
        done
      done
    done
  done
done

rm temp.log
echo ">>> FIN BARRIDO MATRICIAL <<<"