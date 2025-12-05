#!/bin/bash
#SBATCH --job-name=opt-sweep
#SBATCH --output=logs/opt_%j.out
#SBATCH --error=logs/opt_%j.err
#SBATCH --partition=stan
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=112

# Cargar entorno
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# Configuración
EXE="./out/build/default/par/Release/render-par"
SCENE="res/scene_scripts/scene5.txt"
CONFIG="res/config_scripts/config5.txt"
OUTPUT_IMG="out_opt.ppm"
RESULT_FILE="logs/results_optimization.csv"

# Crear cabecera CSV
echo "Partitioner,GrainSize,Time(s),Energy(J)" > $RESULT_FILE

echo ">>> INICIANDO BARRIDO DE OPTIMIZACIÓN <<<"
echo ">>> Rama analysis/writer: Solo escritura PPM es paralela <<<"

# Número de hilos fijo para este barrido (profesor recomienda máximo)
FIXED_THREADS=56

# Estrategias a probar
PARTITIONERS=("auto" "simple" "static" "affinity")
# Tamaños de grano para WRITER: Granos medianos/grandes (la tarea es muy rápida)
# Empezar con grain=threads, luego valores típicos de I/O
GRAINS=(56 28 14 7 1 0 100 500 1000 5000)

for PART in "${PARTITIONERS[@]}"; do
    for GRAIN in "${GRAINS[@]}"; do
        
        # simple_partitioner falla con grain=0, lo saltamos
        if [ "$PART" == "simple" ] && [ "$GRAIN" -eq 0 ]; then continue; fi

        echo "Probando: $PART | Grain: $GRAIN"
        
        # Ejecutamos con configuración de WRITER paralelo y threads fijos
        # En esta rama, el writer paralelo usa --image-part y --image-grain
        perf stat -e power/energy-pkg/ -o temp.log $EXE $SCENE $CONFIG $OUTPUT_IMG --image-part $PART --image-grain $GRAIN --threads $FIXED_THREADS
        
        # Extraer datos (Adaptado a tu salida exacta)
        # Asumiendo que tu programa imprime "X.XXXX seconds time elapsed"
        TIME=$(grep "seconds time elapsed" temp.log | awk '{print $1}' | tr ',' '.')
        ENERGY=$(grep "Joules" temp.log | awk '{print $1}' | tr ',' '.')
        
        # Guardar si obtuvimos datos válidos
        if [ ! -z "$TIME" ]; then
            echo "$PART,$GRAIN,$TIME,$ENERGY" >> $RESULT_FILE
        fi
    done
done

rm temp.log
echo ">>> FIN OPTIMIZACIÓN <<<"
echo ">>> NOTA: Mejoras mínimas esperadas (escritura PPM <5% del tiempo total) <<<"
