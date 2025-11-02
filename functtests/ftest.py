import subprocess
from pathlib import Path
import tempfile
import sys
import os
import traceback
import argparse

# --- CONTENIDO DE ARCHIVOS VÁLIDOS (PARA USAR COMO BASE) ---
VALID_CONFIG_CONTENT = """
aspect_ratio: 16 9
image_width: 100
gamma: 2.2
"""
VALID_SCENE_CONTENT = "matte: default_mat 1 1 1\nsphere: 0 0 0 1 default_mat\n"

# --- FUNCIÓN HELPER PARA EJECUTAR EL PROGRAMA ---
def run_executable(executable_path, scene_path, config_path, output_path):
    """Ejecuta el programa C++ y captura su salida."""
    command = [executable_path, str(scene_path), str(config_path), str(output_path)]
    if not os.path.exists(executable_path):
        raise FileNotFoundError(
            f"El ejecutable no se encontró en '{executable_path}'."
        )
    result = subprocess.run(command, capture_output=True, text=True, encoding='utf-8')
    return result.stderr, result.returncode

# --- DEFINICIÓN DE LAS PRUEBAS ---

def test_success_valid_files(executable_path):
    """FICHERO DE ESTUDIO: ambos. OBJETO DE PRUEBA: Escena y configuración validas."""
    with tempfile.TemporaryDirectory() as temp_dir:
        d = Path(temp_dir)
        config_path, scene_path, output_path = d / "c.txt", d / "s.txt", d / "o.ppm"
        scene_from_spreadsheet = """
            matte: mat1 1.0 0.3 0.3; matte: mat2 0.3 1.0 0.3; matte: mat3 0.3 0.3 1.0;
            matte: mat4 0.5 0.5 0.5; metal: metal1 0.9 0.9 0.9 0.05; refractive: ref1 1.3;
            sphere: 5 5 0 3 mat2; sphere: -5 5 0 3 ref1; sphere: -5 -5 0 3 mat3;
            sphere: 5 -5 0 3 metal1; sphere: 0 -500 0 490 mat4; sphere: -2 5 4 3 mat1;
            cylinder: 5 5 0 0.8 0 3 0 mat4; cylinder: -8 6.5 0 0.6 4 0 0 mat1;
            cylinder: 7 1 0 1.0 -1 5 10 metal1; cylinder: 8 3 -5 0.5 0 0 3 mat2;
        """.replace(';', '\n')
        config_path.write_text(VALID_CONFIG_CONTENT)
        scene_path.write_text(scene_from_spreadsheet)
        stderr, code = run_executable(executable_path, scene_path, config_path, output_path)
        assert code == 0, f"El programa falló con archivos válidos (código {code}).\n    STDERR OBTENIDO:\n---\n{stderr.strip()}\n---"
        assert stderr == "", f"Se esperaba un stderr vacío, pero se obtuvo:\n---\n{stderr.strip()}\n---"

def test_config_unrecognized_keys(executable_path):
    """FICHERO DE ESTUDIO: configuracion. OBJETO DE PRUEBA: Etiquetas no reconocidas."""
    cases = [
        ("image_widt:", "Error: Unknown configuration key: [image_widt:]"),
        ("camera_target 0 0 0", "Error: Unknown configuration key: [camera_target]")
    ]
    for line, expected_err in cases:
        with tempfile.TemporaryDirectory() as temp_dir:
            d = Path(temp_dir)
            config_path, scene_path, output_path = d / "c.txt", d / "s.txt", d / "o.ppm"
            config_path.write_text(line + "\n")
            scene_path.write_text(VALID_SCENE_CONTENT)
            stderr, code = run_executable(executable_path, scene_path, config_path, output_path)
            assert code != 0, f"Caso '{line}' tuvo éxito, pero se esperaba un error."
            assert expected_err in stderr, f"Caso '{line}' no produjo el error esperado.\n    ESPERADO CONTENER: '{expected_err}'\n    STDERR OBTENIDO: '{stderr.strip()}'"

def test_config_invalid_values(executable_path):
    """FICHERO DE ESTUDIO: configuracion. OBJETO DE PRUEBA: Valores invalidos."""
    cases = [
        "aspect_ratio: 1 0", "aspect_ratio: 0 1", "aspect_ratio: invalido", "image_width: 0",
        "field_of_view: 0", "field_of_view: 180", "samples_per_pixel: 0", "max_depth: 0",
        "ray_rng_seed: 0", "material_rng_seed: 0", "background_dark_color: 1.1 0.5 1",
        "background_light_color: 1 1.1 1"
    ]
    for line in cases:
        with tempfile.TemporaryDirectory() as temp_dir:
            d = Path(temp_dir)
            config_path, scene_path, output_path = d / "c.txt", d / "s.txt", d / "o.ppm"
            config_path.write_text(line + "\n")
            scene_path.write_text(VALID_SCENE_CONTENT)
            stderr, code = run_executable(executable_path, scene_path, config_path, output_path)
            key = line.split()[0]
            expected_err_1 = f"Error: Invalid value for key: [{key}]"
            expected_err_2 = f"Line: \"{line}\""
            assert code != 0, f"Caso '{line}' tuvo éxito, pero se esperaba un error."
            assert expected_err_1 in stderr, f"Caso '{line}'.\n    ESPERADO CONTENER: '{expected_err_1}'\n    STDERR OBTENIDO: '{stderr.strip()}'"
            assert expected_err_2 in stderr, f"Caso '{line}'.\n    ESPERADO CONTENER: '{expected_err_2}'\n    STDERR OBTENIDO: '{stderr.strip()}'"

def test_scene_unrecognized_entity(executable_path):
    """FICHERO DE ESTUDIO: descripción. OBJETO DE PRUEBA: etiquetas no reconocidas."""
    with tempfile.TemporaryDirectory() as temp_dir:
        d = Path(temp_dir)
        config_path, scene_path, output_path = d / "c.txt", d / "s.txt", d / "o.ppm"
        config_path.write_text(VALID_CONFIG_CONTENT)
        scene_path.write_text("pyramid:\n")
        stderr, code = run_executable(executable_path, scene_path, config_path, output_path)
        expected_err = "Error: Unknown scene entity: pyramid"
        assert code != 0, "El programa tuvo éxito con una entidad desconocida, pero se esperaba un error."
        assert expected_err in stderr, f"No se produjo el error esperado.\n    ESPERADO CONTENER: '{expected_err}'\n    STDERR OBTENIDO: '{stderr.strip()}'"

def test_scene_insufficient_info(executable_path):
    """FICHERO DE ESTUDIO: descripción. OBJETO DE PRUEBA: información insuficiente."""
    cases = [
        ("matte: mat 0.8 0.1", "Error: Invalid matte parameters"),
        ("metal: met 0.8 0.2 1", "Error: Invalid metal parameters"),
        ("refractive: ref", "Error: Invalid refractive parameters"),
        ("sphere: 0 1 3", "Error: Invalid sphere parameters"),
        ("cylinder: 0 1 4 3 6 -6 met1", "Error: Invalid cylinder parameters"),
    ]
    for line, expected_err in cases:
        with tempfile.TemporaryDirectory() as temp_dir:
            d = Path(temp_dir)
            config_path, scene_path, output_path = d / "c.txt", d / "s.txt", d / "o.ppm"
            config_path.write_text(VALID_CONFIG_CONTENT)
            scene_path.write_text("metal: met1 1 1 1 0\n" + line + "\n")
            stderr, code = run_executable(executable_path, scene_path, config_path, output_path)
            assert code != 0, f"Caso '{line}' tuvo éxito, pero se esperaba un error."
            assert expected_err in stderr, f"Caso '{line}'.\n    ESPERADO CONTENER: '{expected_err}'\n    STDERR OBTENIDO: '{stderr.strip()}'"
            assert f"Line: \"{line}\"" in stderr, f"Caso '{line}'.\n    ESPERADO CONTENER LA LÍNEA DEL ERROR.\n    STDERR OBTENIDO: '{stderr.strip()}'"

def test_scene_excessive_info(executable_path):
    """FICHERO DE ESTUDIO: descripción. OBJETO DE PRUEBA: información excesiva."""
    cases = [
        ("refractive: ref 0.3 2", "refractive:", "\"2\""),
        ("sphere: 0 1 3 5 mat1 7", "sphere:", "\"7\""),
        ("cylinder: 0 0 0 0.5 20 10 -5 metal1 ref", "cylinder:", "\"ref\""),
    ]
    for line, key, extra in cases:
        with tempfile.TemporaryDirectory() as temp_dir:
            d = Path(temp_dir)
            config_path, scene_path, output_path = d / "c.txt", d / "s.txt", d / "o.ppm"
            config_path.write_text(VALID_CONFIG_CONTENT)
            scene_path.write_text("matte: mat1 1 1 1\nmetal: metal1 1 1 1 0\n" + line + "\n")
            stderr, code = run_executable(executable_path, scene_path, config_path, output_path)
            expected_err_1 = f"Error: Extra data after configuration value for key: [{key}]"
            expected_err_2 = f"Extra: {extra}"
            expected_err_3 = f"Line: \"{line}\""
            assert code != 0, f"Caso '{line}' tuvo éxito, pero se esperaba un error."
            assert expected_err_1 in stderr, f"Caso '{line}'.\n    ESPERADO CONTENER: '{expected_err_1}'\n    STDERR OBTENIDO: '{stderr.strip()}'"
            assert expected_err_2 in stderr, f"Caso '{line}'.\n    ESPERADO CONTENER: '{expected_err_2}'\n    STDERR OBTENIDO: '{stderr.strip()}'"
            assert expected_err_3 in stderr, f"Caso '{line}'.\n    ESPERADO CONTENER: '{expected_err_3}'\n    STDERR OBTENIDO: '{stderr.strip()}'"

def test_scene_invalid_info(executable_path):
    """FICHERO DE ESTUDIO: descripción. OBJETO DE PRUEBA: información invalida."""
    cases = [
        ("matte: mat 8 a 7", "Error: Invalid matte parameters"),
        ("metal: met 3 4 5 b", "Error: Invalid metal parameters"),
        ("refractive: ref z", "Error: Invalid refractive parameters"),
        ("sphere: -5 6 7 0 mat1", "Error: Invalid sphere parameters"),
        ("cylinder: 0 3 4 0 6 7 8 ref1", "Error: Invalid cylinder parameters"),
    ]
    for line, expected_err in cases:
        with tempfile.TemporaryDirectory() as temp_dir:
            d = Path(temp_dir)
            config_path, scene_path, output_path = d / "c.txt", d / "s.txt", d / "o.ppm"
            config_path.write_text(VALID_CONFIG_CONTENT)
            scene_path.write_text("matte: mat1 1 1 1\nrefractive: ref1 1.3\n" + line + "\n")
            stderr, code = run_executable(executable_path, scene_path, config_path, output_path)
            assert code != 0, f"Caso '{line}' tuvo éxito, pero se esperaba un error."
            assert expected_err in stderr, f"Caso '{line}'.\n    ESPERADO CONTENER: '{expected_err}'\n    STDERR OBTENIDO: '{stderr.strip()}'"
            assert f"Line: \"{line}\"" in stderr, f"Caso '{line}'.\n    ESPERADO CONTENER LA LÍNEA DEL ERROR.\n    STDERR OBTENIDO: '{stderr.strip()}'"

def test_scene_duplicate_material(executable_path):
    """FICHERO DE ESTUDIO: descripción. OBJETO DE PRUEBA: materiales repetidos."""
    line = "matte: matA 0.1 0.2 0.3"
    with tempfile.TemporaryDirectory() as temp_dir:
        d = Path(temp_dir)
        config_path, scene_path, output_path = d / "c.txt", d / "s.txt", d / "o.ppm"
        config_path.write_text(VALID_CONFIG_CONTENT)
        scene_path.write_text("matte: matA 0.8 0.1 0.7\n" + line + "\n")
        stderr, code = run_executable(executable_path, scene_path, config_path, output_path)
        expected_err_1 = "Error: Material with name [matA] already exists"
        expected_err_2 = f"Line: \"{line}\""
        assert code != 0, "El programa tuvo éxito con material duplicado, pero se esperaba un error."
        assert expected_err_1 in stderr, f"No se produjo el error de material duplicado.\n    ESPERADO CONTENER: '{expected_err_1}'\n    STDERR OBTENIDO: '{stderr.strip()}'"
        assert expected_err_2 in stderr, f"No se mostró la línea del error para material duplicado.\n    ESPERADO CONTENER: '{expected_err_2}'\n    STDERR OBTENIDO: '{stderr.strip()}'"

def test_scene_material_not_found(executable_path):
    """FICHERO DE ESTUDIO: descripción. OBJETO DE PRUEBA: materiales no existentes."""
    line = "sphere: -5 6 7 10 inex"
    with tempfile.TemporaryDirectory() as temp_dir:
        d = Path(temp_dir)
        config_path, scene_path, output_path = d / "c.txt", d / "s.txt", d / "o.ppm"
        config_path.write_text(VALID_CONFIG_CONTENT)
        scene_path.write_text(line + "\n")
        stderr, code = run_executable(executable_path, scene_path, config_path, output_path)
        expected_err_1 = "Error: Material not found: [inex]"
        expected_err_2 = f"Line: \"{line}\""
        assert code != 0, "El programa tuvo éxito con material no encontrado, pero se esperaba un error."
        assert expected_err_1 in stderr, f"No se produjo el error de material no encontrado.\n    ESPERADO CONTENER: '{expected_err_1}'\n    STDERR OBTENIDO: '{stderr.strip()}'"
        assert expected_err_2 in stderr, f"No se mostró la línea del error para material no encontrado.\n    ESPERADO CONTENER: '{expected_err_2}'\n    STDERR OBTENIDO: '{stderr.strip()}'"

# --- TEST RUNNER MANUAL ---
def main():
    """Analiza los argumentos, ejecuta todas las pruebas y reporta un resumen."""
    
    parser = argparse.ArgumentParser(
        description="Suite de pruebas funcionales para el renderizador."
    )
    parser.add_argument(
        'executable_path',
        help="Ruta al ejecutable del renderizador que se va a probar."
    )
    args = parser.parse_args()

    executable_path = args.executable_path

    # Eliminamos la prueba de valores por defecto que no estaba en la hoja de cálculo
    tests_to_run = [
        test_success_valid_files,
        test_config_unrecognized_keys,
        test_config_invalid_values,
        test_scene_unrecognized_entity,
        test_scene_insufficient_info,
        test_scene_excessive_info,
        test_scene_invalid_info,
        test_scene_duplicate_material,
        test_scene_material_not_found,
    ]
    
    passed_count = 0
    failed_count = 0
    
    print("==================== Running Functional Tests ====================")
    for test_func in tests_to_run:
        test_name = test_func.__name__
        print(f"Running: {test_name}...")
        try:
            test_func(executable_path)
            print("  -> \033[92mPASSED\033[0m")
            passed_count += 1
        except AssertionError as e:
            print(f"  -> \033[91mFAILED\033[0m")
            print(f"    \033[91mReason: {e}\033[0m")
            failed_count += 1
        except Exception as e:
            print(f"  -> \033[91mERROR: An unexpected exception occurred.\033[0m")
            print(f"    \033[91mDetails: {e}\033[0m")
            print("    \033[90m--- Traceback ---")
            for line in traceback.format_exc().splitlines():
                print(f"    {line}")
            print("    ---\033[0m")
            failed_count += 1
            
    print("\n======================= Test Summary =======================")
    print(f"Total tests: {len(tests_to_run)}")
    print(f"\033[92mPASSED: {passed_count}\033[0m")
    print(f"\033[91mFAILED: {failed_count}\033[0m")
    print("==========================================================")
    
    if failed_count > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()