#!/bin/bash
echo "EJECUTANDO EN AVIGNON 5 SBATCH"

run_scripts=("res/run/run1.sh" "res/run/run2.sh" "res/run/run3.sh" "res/run/run4.sh")
declare -a job_ids

echo "Enviando trabajos de renderizado..."
for script in "${run_scripts[@]}"; do
  output=$(sbatch "$script")
  job_id=$(echo "$output" | cut -d ' ' -f 4)
  echo " -> Enviado $script con Job ID: $job_id"
  job_ids+=("$job_id")
done

# Construye la cadena de dependencias para el trabajo final
# El formato es afterok:ID1:ID2:ID3...
# "afterok" significa "ejecutar solo si los trabajos anteriores terminaron sin error"
dependency_string="afterok:$(IFS=:; echo "${job_ids[*]}")"

echo "Enviando el trabajo recolector con dependencia: $dependency_string"

# Envía un trabajo final que depende de todos los anteriores.
# Este trabajo ejecutará el script "show_results.sh".
sbatch --dependency="$dependency_string" res/run/show_results.sh "${job_ids[@]}"

echo "------------------------------------------------------------------"
echo "Todos los trabajos han sido enviados."
echo "Se ha programado un trabajo final para mostrar los resultados cuando terminen."
echo "Puedes cerrar esta terminal. El resultado aparecerá en el archivo slurm-ID.out del trabajo recolector."