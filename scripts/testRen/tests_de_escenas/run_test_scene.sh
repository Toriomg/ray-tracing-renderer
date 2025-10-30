#!/bin/bash
# Script para enviar a la cola (sbatch) TODAS las pruebas de CONFIGURACIÓN.

set -euo pipefail

TEST_DIR="tests_de_config"

# --- Comprobaciones Previas ---
if [ ! -d "${TEST_DIR}" ]; then
    echo "Error: No se encuentra el directorio '${TEST_DIR}'."
    echo "Asegúrate de haber ejecutado primero el script de Python generador."
    exit 1
fi

# --- Lógica Principal ---
echo "================================================="
echo "ENVIANDO PRUEBAS DE CONFIGURACIÓN A LA COLA"
echo "================================================="

SCRIPT_COUNT=$(find "${TEST_DIR}" -type f -name "*.sh" | wc -l)

if [ "$SCRIPT_COUNT" -eq 0 ]; then
    echo "Advertencia: No se encontraron scripts de prueba (*.sh) en '${TEST_DIR}'."
    exit 0
fi

echo "Se encontraron ${SCRIPT_COUNT} scripts de prueba de configuración para enviar."
echo ""

find "${TEST_DIR}" -type f -name "*.sh" -print0 | while IFS= read -r -d $'\0' script; do
    echo "-> Enviando trabajo: $script"
    sbatch "$script"
done

echo ""
echo "================================================="
echo "${SCRIPT_COUNT} TRABAJOS DE CONFIGURACIÓN HAN SIDO ENVIADOS"
echo "================================================="