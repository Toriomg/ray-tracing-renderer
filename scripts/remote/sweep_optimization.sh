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

# Estrategias a probar
PARTITIONERS=("auto" "simple" "static" "affinity")
# Tamaños de grano (0 = auto de TBB, luego potencias de 2)
GRAINS=(0 1 32 64 128 256 512 1024)

for PART in "${PARTITIONERS[@]}"; do
    for GRAIN in "${GRAINS[@]}"; do
        
        # simple_partitioner falla con grain=0, lo saltamos
        if [ "$PART" == "simple" ] && [ "$GRAIN" -eq 0 ]; then continue; fi

        echo "Probando: $PART | Grain: $GRAIN"
        
        # Ejecutar perf y capturar exit code sin abortar
        set +e
        perf stat -r 5 -e power/energy-pkg/ -o temp.log $EXE $SCENE $CONFIG $OUTPUT_IMG --image-part $PART --image-grain $GRAIN 2>&1
        PERF_EXIT=$?
        set -e
        
        # Si perf falló, registrar error y continuar
        if [ $PERF_EXIT -ne 0 ]; then
            echo "ERROR: perf stat falló para $PART / grain=$GRAIN (exit: $PERF_EXIT)" >&2
            echo "$PART,$GRAIN,ERROR,ERROR" >> $RESULT_FILE
            sync
            continue
        fi
        
        # Parsear con protección (|| echo "N/A" evita error fatal de grep)
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

rm temp.log
echo ">>> FIN OPTIMIZACIÓN <<<"