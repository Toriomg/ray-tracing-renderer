#!/bin/bash
#SBATCH --job-name=compile_render
#SBATCH --output=compile_output-%j.out
#SBATCH --error=compile_error-%j.err
#SBATCH --time=00:05:00

echo "--- Iniciando compilación en $(hostname) ---"

# Limpiamos una posible build anterior
rm -rf build

# Configuramos el proyecto con CMake usando el preset para release.
# Esto es crucial para las pruebas de rendimiento.
# CMake buscará el archivo CMakePresets.json
cmake --preset gcc-release

# Compilamos el proyecto
cmake --build --preset gcc-release

echo "--- Compilación finalizada ---"