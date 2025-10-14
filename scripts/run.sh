#!/bin/sh
#SBATCH --job-name=run_render
#SBATCH --output=run_output-%j.out
#SBATCH --error=run_error-%j.err
#SBATCH --time=00:10:00 # Ajusta el tiempo si tu render tarda más

echo "--- Iniciando ejecución en $(hostname) ---"

# Rutas a los archivos de entrada y salida
CONFIG_FILE="res/config_simple.txt"  # Cambia por tu archivo de config
SCENE_FILE="res/scene_simple.txt"    # Cambia por tu archivo de escena
OUTPUT_FILE_SOA="outputImageSOA.ppm"
OUTPUT_FILE_AOS="outputImageAOS.ppm"

# Ejecutamos la versión SOA y medimos el tiempo
echo "--- Ejecutando render-soa ---"
time ./build/soa/render-soa ${CONFIG_FILE} ${SCENE_FILE} ${OUTPUT_FILE_SOA}

# Ejecutamos la versión AOS y medimos el tiempo
echo "--- Ejecutando render-aos ---"
time ./build/aos/render-aos ${CONFIG_FILE} ${SCENE_FILE} ${OUTPUT_FILE_AOS}

echo "--- Ejecución finalizada ---"