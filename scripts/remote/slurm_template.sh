def get_bash_script_template(test_name, config_file, scene_file, out_prefix):
    return f"""#!/bin/bash
#SBATCH --job-name={test_name}
#SBATCH --output={test_name}_%j.out
#SBATCH --error={test_name}_%j.err
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --time=00:10:00
#SBATCH --exclusive  # Importante para mediciones fiables de energía

# Cargar entorno
module load gcc/14.1.0

# Rutas
BASE_DIR=$SLURM_SUBMIT_DIR
RENDER_SOA="$BASE_DIR/out/build/default/soa/Release/render-soa"
RENDER_AOS="$BASE_DIR/out/build/default/aos/Release/render-aos"

CONF="$BASE_DIR/{config_file}"
SCENE="$BASE_DIR/{scene_file}"

# Salidas
OUT_SOA="{out_prefix}_SOA.ppm"
OUT_AOS="{out_prefix}_AOS.ppm"

echo "RESULT_CSV_HEADER: test_name;type;time;energy_j"

# Ejecución SOA con Perf (Energía y Tiempo)
# -r 5: Repetir 5 veces
# -e power/energy-pkg/: Medir energía del paquete CPU
echo ">>> Ejecutando SOA: {test_name}"
perf stat -r 5 -e power/energy-pkg/ -o perf_soa.txt $RENDER_SOA $SCENE $CONF $OUT_SOA

# Ejecución AOS
echo ">>> Ejecutando AOS: {test_name}"
perf stat -r 5 -e power/energy-pkg/ -o perf_aos.txt $RENDER_AOS $SCENE $CONF $OUT_AOS

# Aquí podrías añadir un pequeño parser con awk/grep para imprimir la línea CSV final
# para que tu script de consolidación la capture.
"""