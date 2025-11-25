import os
import sys
import shutil
import random

# --- 1. CONFIGURACIÓN GLOBAL DE LAS PRUEBAS ---
# Todos los parámetros de las pruebas se definen aquí para fácil modificación.

# Ruta base donde se generará todo (el script debe ejecutarse desde aquí).
BASE_PATH = os.getcwd() # Asume que se ejecuta desde /workspace/scripts/testRen

# Rutas a los ejecutables (relativas a BASE_PATH)
RENDER_SOA_EXE = "../../out/build/default/soa/Release/render-soa"
RENDER_AOS_EXE = "../../out/build/default/aos/Release/render-aos"

# --- Parámetros para las pruebas de CONFIGURACIÓN ---
# Se usará una escena fija que se copiará al directorio de pruebas.
ESCENA_FIJA_ORIGEN = os.path.join(BASE_PATH, "../../res/scene_scripts/scene2.txt")
ESCENA_FIJA_DESTINO = "scene_fixed.txt" # Nombre del archivo copiado

# Listas de valores para cada tipo de prueba de configuración
VALORES_PROFUNDIDAD = [5, 10, 20, 50, 100, 200]
VALORES_SAMPLES = [3, 5, 15, 30, 50, 150]
VALORES_ANCHO_IMAGEN = [270, 720, 1280, 1920, 2340, 3840]

# --- Parámetros para las pruebas de ESCENA ---
# Se usará una configuración vacía para forzar valores por defecto.
CONFIG_VACIO_NOMBRE = "config_vacio.txt"

# Número de objetos para las pruebas de escalabilidad.
CANTIDAD_OBJETOS = [1, 2, 4, 8, 16, 32, 128, 256]
TIPOS_MATERIAL = ["matte", "metal", "refractivo"]
TIPOS_OBJETO = ["esfera", "cilindro"]

# --- Constantes para el generador de escenas (del script anterior) ---
SEED_FIJA = 42
RANGO_POS_X = (-10.0, 10.0); RANGO_POS_Y = (-6.0, 8.0); RANGO_POS_Z = (-5.0, 15.0)
RANGO_RADIO = (0.2, 0.8); RANGO_EJE_CILINDRO = (-3.0, 3.0)
RANGO_COLOR_RGB = (0.1, 1.0); RANGO_DIFUSION_METAL = (0.0, 0.75)
RANGO_INDICE_REFRACCION = (1.3, 2.0)

# --- 2. PLANTILLAS Y FUNCIONES DE GENERACIÓN ---

def get_bash_script_template(test_name, config_file, scene_file, out_prefix):
    """Genera el contenido de un script de Bash para una prueba específica."""
    output_dir = os.path.dirname(config_file)
    output_prefix_with_path = os.path.join(output_dir, out_prefix)
    return f"""#!/bin/bash
# Script de prueba autogenerado para: {test_name}

set -Eeuo pipefail
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${{LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}}"

echo "--- Iniciando prueba '{test_name}' en $(hostname) ---"

# Rutas a los archivos de entrada y salida
CONFIG_FILE="{config_file}"
SCENE_FILE="{scene_file}"
OUTPUT_FILE_SOA="{output_prefix_with_path}_SOA.ppm"
OUTPUT_FILE_AOS="{output_prefix_with_path}_AOS.ppm"

# Ruta a los ejecutables
RENDER_SOA_EXE="{RENDER_SOA_EXE}"
RENDER_AOS_EXE="{RENDER_AOS_EXE}"

# Comando de medición con energía
PERF_COMMAND="perf stat -r 3 -e power/energy-pkg/"

# --- Medición de rendimiento para render-soa ---
echo ""
echo "========================================="
echo ">>> Midiendo 'render-soa' (3 ejecuciones)"
echo "========================================="
$PERF_COMMAND ${{RENDER_SOA_EXE}} ${{SCENE_FILE}} ${{CONFIG_FILE}} ${{OUTPUT_FILE_SOA}}
# ${{RENDER_SOA_EXE}} ${{SCENE_FILE}} ${{CONFIG_FILE}} ${{OUTPUT_FILE_SOA}}

# --- Medición de rendimiento para render-aos ---
echo ""
echo "========================================="
echo ">>> Midiendo 'render-aos' (3 ejecuciones)"
echo "========================================="
$PERF_COMMAND ${{RENDER_AOS_EXE}} ${{SCENE_FILE}} ${{CONFIG_FILE}} ${{OUTPUT_FILE_AOS}}
# ${{RENDER_AOS_EXE}} ${{SCENE_FILE}} ${{CONFIG_FILE}} ${{OUTPUT_FILE_AOS}}

echo "--- Prueba '{test_name}' finalizada. ---"
"""

# Funciones del generador de escenas (integradas aquí para simplicidad)
def generar_escena_completa(tipo_objeto, tipo_material, num_objetos):
    """Genera el contenido completo de un archivo de escena."""
    # Generación de materiales
    random.seed(SEED_FIJA)
    materiales_str = []
    for i in range(num_objetos):
        nombre_material = f"mat_{i}"
        r, g, b = [random.uniform(*RANGO_COLOR_RGB) for _ in range(3)]
        if tipo_material == "matte":
            materiales_str.append(f"matte: {nombre_material} {r:.2f} {g:.2f} {b:.2f}")
        elif tipo_material == "metal":
            difusion = random.uniform(*RANGO_DIFUSION_METAL)
            materiales_str.append(f"metal: {nombre_material} {r:.2f} {g:.2f} {b:.2f} {difusion:.2f}")
        elif tipo_material == "refractivo":
            idx_ref = random.uniform(*RANGO_INDICE_REFRACCION)
            materiales_str.append(f"refractive: {nombre_material} {idx_ref:.2f}")
    
    # Generación de objetos
    random.seed(SEED_FIJA + 1)
    objetos_str = []
    for i in range(num_objetos):
        nombre_mat_asociado = f"mat_{i}"
        cx, cy, cz = random.uniform(*RANGO_POS_X), random.uniform(*RANGO_POS_Y), random.uniform(*RANGO_POS_Z)
        radio = random.uniform(*RANGO_RADIO)
        if tipo_objeto == "esfera":
            objetos_str.append(f"sphere: {cx:.2f} {cy:.2f} {cz:.2f} {radio:.2f} {nombre_mat_asociado}")
        elif tipo_objeto == "cilindro":
            ax, ay, az = [random.uniform(*RANGO_EJE_CILINDRO) for _ in range(3)]
            if ax == 0 and ay == 0 and az == 0: ay = 1.0
            objetos_str.append(f"cylinder: {cx:.2f} {cy:.2f} {cz:.2f} {radio:.2f} {ax:.2f} {ay:.2f} {az:.2f} {nombre_mat_asociado}")

    return "\n".join(materiales_str) + "\n\n" + "\n".join(objetos_str)


# --- 3. LÓGICA PRINCIPAL DE CREACIÓN DEL BANCO DE PRUEBAS ---

def crear_pruebas_config():
    """Crea la estructura y archivos para las pruebas de configuración."""
    print("Generando pruebas de configuración...")
    path = os.path.join(BASE_PATH, "tests_de_config")
    os.makedirs(path, exist_ok=True)
    
    # Copiar la escena fija
    escena_fija_path_abs = os.path.join(path, ESCENA_FIJA_DESTINO)
    shutil.copy(ESCENA_FIJA_ORIGEN, escena_fija_path_abs)
    # **CAMBIO CLAVE**: Calcular la ruta relativa que se usará en los scripts
    escena_fija_path_rel = os.path.relpath(escena_fija_path_abs, BASE_PATH)
    
    all_scripts = []

    # 1. Pruebas de profundidad de rayos
    dir_profundidad = os.path.join(path, "01_profundidad_rayos")
    os.makedirs(dir_profundidad, exist_ok=True)
    for depth in VALORES_PROFUNDIDAD:
        test_name = f"profundidad_{depth}"
        config_file = f"config_{test_name}.txt"
        script_file = f"test_{test_name}.sh"
        
        config_path_abs = os.path.join(dir_profundidad, config_file)

        with open(os.path.join(dir_profundidad, config_file), "w") as f:
            f.write(f"max_depth: {depth}\n")

        config_path_rel = os.path.relpath(config_path_abs, BASE_PATH)
            
        script_content = get_bash_script_template(
            test_name,
            config_path_rel,     
            escena_fija_path_rel,
            test_name
        )
        script_path = os.path.join(dir_profundidad, script_file)
        with open(script_path, "w") as f:
            f.write(script_content)
        os.chmod(script_path, 0o755)
        all_scripts.append(os.path.relpath(script_path, BASE_PATH))

    # 2. Pruebas de samples por píxel
    dir_samples = os.path.join(path, "02_samples_per_pixel")
    os.makedirs(dir_samples, exist_ok=True)
    for samples in VALORES_SAMPLES:
        test_name = f"samples_{samples}"
        config_file = f"config_{test_name}.txt"
        script_file = f"test_{test_name}.sh"

        config_path_abs = os.path.join(dir_samples, config_file)

        with open(os.path.join(dir_samples, config_file), "w") as f:
            f.write(f"samples_per_pixel: {samples}\n")

        config_path_rel = os.path.relpath(config_path_abs, BASE_PATH)

        script_content = get_bash_script_template(
            test_name,
            config_path_rel,     
            escena_fija_path_rel,
            test_name
        )
        script_path = os.path.join(dir_samples, script_file)
        with open(script_path, "w") as f:
            f.write(script_content)
        os.chmod(script_path, 0o755)
        all_scripts.append(os.path.relpath(script_path, BASE_PATH))

    # 3. Pruebas de ancho de imagen
    dir_ancho = os.path.join(path, "03_ancho_imagen")
    os.makedirs(dir_ancho, exist_ok=True)
    for width in VALORES_ANCHO_IMAGEN:
        test_name = f"ancho_{width}"
        config_file = f"config_{test_name}.txt"
        script_file = f"test_{test_name}.sh"

        config_path_abs = os.path.join(dir_ancho, config_file)

        with open(os.path.join(dir_ancho, config_file), "w") as f:
            f.write(f"image_width: {width}\n")

        config_path_rel = os.path.relpath(config_path_abs, BASE_PATH)

        script_content = get_bash_script_template(
            test_name,
            config_path_rel,     
            escena_fija_path_rel,
            test_name
        )
        script_path = os.path.join(dir_ancho, script_file)
        with open(script_path, "w") as f:
            f.write(script_content)
        os.chmod(script_path, 0o755)
        all_scripts.append(os.path.relpath(script_path, BASE_PATH))
        
    return all_scripts

def crear_pruebas_escena():
    """Crea la estructura y archivos para las pruebas de escena."""
    print("Generando pruebas de escena...")
    path = os.path.join(BASE_PATH, "tests_de_escenas")
    os.makedirs(path, exist_ok=True)

    # --- Manejo del archivo de configuración vacío (se hace UNA SOLA VEZ) ---
    # 1. Definir la ruta absoluta del archivo de configuración.
    config_vacio_path_abs = os.path.join(path, CONFIG_VACIO_NOMBRE)
    # 2. Crear y escribir el archivo usando esa ruta.
    with open(config_vacio_path_abs, "w") as f:
        f.write("# Archivo de configuracion vacio para usar valores por defecto\n")
    # 3. Calcular su ruta relativa a la base, que se usará en todos los scripts.
    config_vacio_path_rel = os.path.relpath(config_vacio_path_abs, BASE_PATH)

    all_scripts = []

    test_idx = 1
    for material in TIPOS_MATERIAL:
        for objeto in TIPOS_OBJETO:
            dir_name = f"{test_idx:02d}_{material}_{objeto}"
            dir_path = os.path.join(path, dir_name)
            os.makedirs(dir_path, exist_ok=True)

            for n_obj in CANTIDAD_OBJETOS:
                test_name = f"{n_obj}_obj"
                scene_filename = f"scene_{test_name}.txt" # Renombrado para más claridad
                script_file = f"test_{test_name}.sh"

                # --- Manejo del archivo de escena específico para esta prueba ---
                # 1. **CAMBIO CLAVE**: Definir la ruta absoluta del archivo de escena.
                scene_path_abs = os.path.join(dir_path, scene_filename)

                # 2. Generar y escribir el contenido de la escena en esa ruta.
                scene_content = generar_escena_completa(objeto, material, n_obj)
                with open(scene_path_abs, "w") as f:
                    f.write(scene_content)

                # 3. **CAMBIO CLAVE**: Ahora que 'scene_path_abs' existe, calcular su ruta relativa.
                scene_path_rel = os.path.relpath(scene_path_abs, BASE_PATH)

                # 4. **CAMBIO CLAVE**: Generar el script de bash pasando las rutas relativas CORRECTAS.
                script_content = get_bash_script_template(
                    f"{material}_{objeto}_{test_name}",
                    config_vacio_path_rel,  # La ruta relativa del config vacío
                    scene_path_rel,         # La ruta relativa de la escena que acabamos de crear
                    test_name
                )
                
                # --- El resto del código para crear el script ya estaba bien ---
                script_path = os.path.join(dir_path, script_file)
                with open(script_path, "w") as f:
                    f.write(script_content)
                os.chmod(script_path, 0o755)
                all_scripts.append(os.path.relpath(script_path, BASE_PATH))

            test_idx += 1

    return all_scripts

def crear_runner_global(all_scripts):
    """Crea el script run_all_tests.sh que ejecuta todas las pruebas en orden."""
    print("Generando el script de ejecución global...")
    runner_path = os.path.join(BASE_PATH, "run_all_tests.sh")
    
    script_list_str = "\n".join([f'    bash "{script}"' for script in all_scripts])
    
    content = f"""#!/bin/bash
# Script para ejecutar todas las pruebas de rendimiento y energía generadas.

set -Eeuo pipefail

echo "================================================="
echo "INICIANDO BANCO DE PRUEBAS DE RENDIMIENTO Y ENERGÍA"
echo "================================================="

# Ejecutar todas las pruebas una por una
{script_list_str}

echo "================================================="
echo "BANCO DE PRUEBAS FINALIZADO"
echo "================================================="
"""
    with open(runner_path, "w") as f:
        f.write(content)
    os.chmod(runner_path, 0o755)
    print(f"\n¡Listo! Para ejecutar todas las pruebas, usa: ./run_all_tests.sh")


if __name__ == "__main__":
    # Comprobar que el script se ejecuta en el directorio esperado
    if not os.path.basename(os.getcwd()) == "testRen":
        print("Advertencia: Este script espera ser ejecutado desde '/workspace/scripts/testRen'.")
        sys.exit(1) # Descomentar para forzar la ejecución en el directorio correcto.

    # Limpiar directorios antiguos si existen
    if os.path.exists("tests_de_config"): shutil.rmtree("tests_de_config")
    if os.path.exists("tests_de_escenas"): shutil.rmtree("tests_de_escenas")
    if os.path.exists("run_all_tests.sh"): os.remove("run_all_tests.sh")

    # Generar todo
    scripts_config = crear_pruebas_config()
    scripts_escena = crear_pruebas_escena()
    crear_runner_global(scripts_config + scripts_escena)
    
    print("\nEstructura de pruebas generada con éxito.")