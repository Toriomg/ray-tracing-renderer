#!/bin/bash
# SLURM_JOB_ID: El ID de este mismo trabajo recolector.
# "$@": Los argumentos pasados desde el sbatch (nuestros IDs originales).

echo "--- Trabajo recolector (Job ID: $SLURM_JOB_ID) iniciado ---"
echo "Todos los trabajos de renderizado han finalizado."
echo "Mostrando sus resultados..."
echo ""

job_ids=("$@")

for job_id in "${job_ids[@]}"; do
  output_file="slurm-${job_id}.out"
  echo "--- Contenido de $output_file ---"
  
  if [ -f "$output_file" ]; then
    cat "$output_file"
  else
    echo "AVISO: No se encontró el archivo de salida $output_file."
  fi
  echo "-------------------------------------------"
  echo ""
done