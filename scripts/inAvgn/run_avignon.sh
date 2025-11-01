#!/bin/bash
# Script para ENVIAR A LA COLA (sbatch) todos los scripts de prueba generados.

# Salir inmediatamente si un comando falla.
set -euo pipefail

# --- Comprobaciones Previas ---
# Asegurarse de que el comando sbatch existe
#if ! command -v sbatch &> /dev/null; then
#    echo "Error: El comando 'sbatch' no se encuentra. ¿Estás en un nodo de acceso de un clúster Slurm?"
#    exit 1
#fi

# Asegurarse de que los directorios de pruebas existen
if [ ! -d "tests_de_config" ] || [ ! -d "tests_de_escenas" ]; then
    echo "Error: No se encuentran los directorios 'tests_de_config' y/o 'tests_de_escenas'."
    echo "Asegúrate de haber ejecutado primero el script de Python 'crear_banco_de_pruebas.py'."
    exit 1
fi

# --- Lógica Principal ---
echo "================================================="
echo "ENVIANDO TODAS LAS PRUEBAS A LA COLA DE SLURM"
echo "================================================="

# Contar cuántos scripts se van a enviar
SCRIPT_COUNT=$(find tests_de_config tests_de_escenas -type f -name "*.sh" | wc -l)

if [ "$SCRIPT_COUNT" -eq 0 ]; then
    echo "Advertencia: No se encontraron scripts de prueba (*.sh) para enviar."
    exit 0
fi

echo "Se encontraron ${SCRIPT_COUNT} scripts de prueba para enviar."
echo ""

# Usar 'find' para localizar todos los scripts .sh en los directorios de pruebas
# y ejecutar 'sbatch' para cada uno.
# -print0 y read -d '' manejan de forma segura nombres de archivo con espacios (aunque no debería haber).
find tests_de_config tests_de_escenas -type f -name "*.sh" -print0 | while IFS= read -r -d $'\0' script; do
    echo "-> Enviando trabajo: $script"
    sbatch "$script"
    #bash "$script"
done

echo ""
echo "================================================="
echo "TODOS LOS ${SCRIPT_COUNT} TRABAJOS HAN SIDO ENVIADOS"
echo "================================================="
echo "Para monitorizar el estado, usa el comando: squeue -u \$USER"
echo "Una vez que todos los trabajos hayan finalizado, ejecuta './recopilar_resultados.sh' para unificar los datos."