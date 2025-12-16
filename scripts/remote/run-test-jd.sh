#!/bin/bash
#SBATCH --job-name=run-test-jd
#SBATCH --output=./logs/txt/run-test-jd.out
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1

set -Eeuo pipefail

# Exportar librerías
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# Ensure directories exist (in case they weren't created by make)
mkdir -p logs/images logs/txt

for i in {1..5}
do
    echo "[ $i ] Iniciando ejecución $i en $(hostname)"

    # Rutas a los archivos de entrada
    CONFIG_FILE="res/config_scripts/config${i}.txt"  
    SCENE_FILE="res/scene_scripts/scene${i}.txt"    

    # Output image directly to logs/images/
    OUTPUT_FILE="logs/images/out${i}.ppm"

    # Ruta al ejecutable
    RENDER_EXE="./out/build/default/par/Release/render-par"

    echo "========================================="
    echo ">>> Midiendo Escena $i"
    echo "========================================="
    
    # Ejecutar medición
    perf stat -r 1 ${RENDER_EXE} ${CONFIG_FILE} ${SCENE_FILE} ${OUTPUT_FILE}

    echo "[ $i ] Mediciones finalizadas."
done

echo "--- Ejecución finalizada ---"
