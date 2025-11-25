#!/bin/bash

# Script para recopilar todos los resultados de los archivos de salida de Slurm
# y unificarlos en un único archivo CSV.

set -euo pipefail

OUTPUT_FILE="resultados_consolidados.csv"
SLURM_FILES_PATTERN="slurm-*.out"

echo "Buscando archivos de Slurm con el patrón: ${SLURM_FILES_PATTERN}"

# Comprobar si existen archivos de salida
if ! ls ${SLURM_FILES_PATTERN} 1> /dev/null 2>&1; then
    echo "Error: No se encontraron archivos de salida de Slurm (${SLURM_FILES_PATTERN})."
    echo "Asegúrate de que los trabajos de Slurm han finalizado."
    exit 1
fi

# Crear la cabecera del archivo CSV final
# Formato de perf -x: valor;unidad;evento;varianza;...
# Nuestro formato: test_name;render_type;valor;unidad;evento;varianza
echo "test_name;render_type;valor;unidad;evento;varianza_porcentual;tiempo_total_seg" > "${OUTPUT_FILE}"

# Buscar en todos los archivos slurm-*.out las líneas que hemos etiquetado
# y procesarlas para construir el CSV final.
grep "RESULT_CSV" ${SLURM_FILES_PATTERN} | while IFS=';' read -r tag test_name render_type val unit event var other; do
    # Eliminar el prefijo "RESULT_CSV:" del primer campo y cualquier espacio
    test_name_clean=$(echo "$test_name" | sed 's/RESULT_CSV://' | xargs)
    
    # La última columna es el tiempo total de ejecución, que es útil
    total_time=$(echo "$other" | grep -o -E '[0-9,.]+ seconds time elapsed' | sed 's/ seconds time elapsed//' | sed 's/,/./')

    echo "${test_name_clean};${render_type};${val};${unit};${event};${var};${total_time}" >> "${OUTPUT_FILE}"
done

echo "¡Recopilación finalizada!"
echo "Resultados guardados en: ${OUTPUT_FILE}"