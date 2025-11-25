#!/bin/bash
# Script para compilar y ejecutar todas las pruebas unitarias del proyecto
# Utiliza CMake Presets y CTest para una ejecución robusta

set -e  # Salir inmediatamente si cualquier comando falla

# Colores para output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}=====================================${NC}"
echo -e "${BLUE}  Ejecutando Pruebas Unitarias${NC}"
echo -e "${BLUE}=====================================${NC}"

# Asegurarnos de estar en el directorio raíz del proyecto
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

echo -e "\n${GREEN}[1/4]${NC} Configurando proyecto con preset 'test'..."
cmake --preset test

echo -e "\n${GREEN}[2/4]${NC} Compilando proyecto..."
cmake --build out/build/test --config Debug

echo -e "\n${GREEN}[3/4]${NC} Navegando al directorio de build..."
cd out/build/test

echo -e "\n${GREEN}[4/4]${NC} Ejecutando todas las pruebas con CTest..."
echo -e "${BLUE}------------------------------------${NC}"
ctest --output-on-failure --build-config Debug

# Verificar el resultado
if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}=====================================${NC}"
    echo -e "${GREEN}  ✓ Todas las pruebas pasaron${NC}"
    echo -e "${GREEN}=====================================${NC}"
else
    echo -e "\n${RED}=====================================${NC}"
    echo -e "${RED}  ✗ Algunas pruebas fallaron${NC}"
    echo -e "${RED}=====================================${NC}"
    exit 1
fi
