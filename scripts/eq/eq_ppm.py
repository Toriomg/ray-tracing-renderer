import sys
import math

# --- Umbrales de aceptación definidos en la especificación ---
MAX_DIFF_THRESHOLD = 150.0
RMSE_CUSTOM_THRESHOLD = 10.0

def parse_ppm_p3(filepath):
    """
    Analiza un archivo de imagen en formato PPM P3 (texto plano).
    Devuelve las dimensiones y una lista plana de valores de píxeles [r,g,b,r,g,b,...].
    """
    try:
        with open(filepath, 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"Error: No se pudo encontrar el archivo '{filepath}'")
        return None, None, None

    # Filtrar comentarios y líneas vacías
    lines = [line.strip() for line in lines if not line.startswith('#') and line.strip()]

    # 1. Magic number (debe ser P3)
    if lines[0] != 'P3':
        print(f"Error: El archivo '{filepath}' no es un formato PPM P3 válido.")
        return None, None, None

    # 2. Dimensiones (ancho y alto)
    try:
        width, height = map(int, lines[1].split())
    except (ValueError, IndexError):
        print(f"Error: Formato de dimensiones inválido en '{filepath}'")
        return None, None, None
    
    # 3. Valor máximo de color (generalmente 255)
    # No lo usaremos en los cálculos, pero es parte del formato
    try:
        max_val = int(lines[2])
    except (ValueError, IndexError):
        print(f"Error: Formato de valor máximo de color inválido en '{filepath}'")
        return None, None, None

    # 4. Datos de los píxeles
    pixel_data = []
    for line in lines[3:]:
        pixel_data.extend(map(int, line.split()))
    
    # Validar que el número de datos de color sea correcto (ancho * alto * 3)
    if len(pixel_data) != width * height * 3:
        print(f"Error: Los datos de píxeles en '{filepath}' no coinciden con las dimensiones {width}x{height}.")
        return None, None, None

    return width, height, pixel_data


def compare_images(width, height, data1, data2):
    """
    Compara dos conjuntos de datos de píxeles según las reglas especificadas.
    """
    max_pixel_difference = 0.0
    sum_of_squares = 0.0
    num_pixels = width * height

    # Iteramos sobre cada píxel (avanzando de 3 en 3 en la lista: r, g, b)
    for i in range(0, len(data1), 3):
        r1, g1, b1 = data1[i:i+3]
        r2, g2, b2 = data2[i:i+3]

        # 1. Cálculo de la diferencia para un píxel (Manhattan distance / 3)
        # Nota: Corrijo el posible error tipográfico en el enunciado "abs(g1,g2)" por "abs(g1-g2)"
        pixel_diff = (abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2)) / 3.0

        # Actualizamos el valor máximo de diferencia encontrado hasta ahora
        if pixel_diff > max_pixel_difference:
            max_pixel_difference = pixel_diff

        # Acumulamos el cuadrado de la diferencia para el cálculo del error
        sum_of_squares += pixel_diff ** 2
    
    # 2. Cálculo del "error cuadrático medio" según la descripción
    # Ojo: La descripción no es el RMSE estándar, es la raíz de la suma de cuadrados.
    # Lo implementamos tal como se describe.
    rmse_custom = math.sqrt(sum_of_squares)

    return max_pixel_difference, rmse_custom

def main():
    """
    Función principal del script.
    """
    # --- Verificación de argumentos de entrada ---
    if len(sys.argv) != 3:
        print("Uso: python comparador_ppm.py <imagen1.ppm> <imagen2.ppm>")
        sys.exit(1)

    file1 = sys.argv[1]
    file2 = sys.argv[2]

    print(f"Comparando imágenes:\n  1: {file1}\n  2: {file2}\n")

    # --- Carga y análisis de las imágenes ---
    width1, height1, data1 = parse_ppm_p3(file1)
    if data1 is None:
        sys.exit(1)
        
    width2, height2, data2 = parse_ppm_p3(file2)
    if data2 is None:
        sys.exit(1)

    # --- Verificación de dimensiones ---
    if width1 != width2 or height1 != height2:
        print("Error: Las imágenes no tienen las mismas dimensiones.")
        print(f"  - {file1}: {width1}x{height1}")
        print(f"  - {file2}: {width2}x{height2}")
        sys.exit(1)

    # --- Comparación ---
    max_diff, rmse = compare_images(width1, height1, data1, data2)
    
    # --- Evaluación de los resultados ---
    is_max_diff_ok = max_diff < MAX_DIFF_THRESHOLD
    is_rmse_ok = rmse < RMSE_CUSTOM_THRESHOLD
    is_valid = is_max_diff_ok and is_rmse_ok

    # --- Cálculo del ratio de certeza ---
    # Lo calculamos como el margen porcentual restante hasta el umbral.
    # Un valor más cercano al 100% es mejor (más lejos del límite).
    # Se usa max(0, ...) para evitar valores negativos si se supera el umbral.
    certainty_diff = max(0, (1 - (max_diff / MAX_DIFF_THRESHOLD))) * 100
    certainty_rmse = max(0, (1 - (rmse / RMSE_CUSTOM_THRESHOLD))) * 100

    # --- Impresión de resultados ---
    print("------ Resultados de la Comparación ------")
    
    # Condición 1: Diferencia máxima
    print(f"\n1. Diferencia Máxima por Píxel:")
    print(f"   - Valor calculado: {max_diff:.4f}")
    print(f"   - Umbral aceptable: < {MAX_DIFF_THRESHOLD}")
    print(f"   - Cumple la condición: {'SÍ' if is_max_diff_ok else 'NO'}")
    print(f"   - Ratio de certeza: {certainty_diff:.2f}%")

    # Condición 2: Error cuadrático medio (según cálculo especificado)
    print(f"\n2. Error Cuadrático Medio (cálculo personalizado):")
    print(f"   - Valor calculado: {rmse:.4f}")
    print(f"   - Umbral aceptable: < {RMSE_CUSTOM_THRESHOLD}")
    print(f"   - Cumple la condición: {'SÍ' if is_rmse_ok else 'NO'}")
    print(f"   - Ratio de certeza: {certainty_rmse:.2f}%")

    # Veredicto final
    print("\n----------------- VEREDICTO -----------------")
    if is_valid:
        print("RESULTADO: VÁLIDO. Ambas imágenes son consideradas aceptables.")
    else:
        print("RESULTADO: NO VÁLIDO. Al menos uno de los umbrales ha sido superado.")
    print("---------------------------------------------")


if __name__ == "__main__":
    main()