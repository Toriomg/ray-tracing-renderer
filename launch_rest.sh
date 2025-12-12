#!/bin/bash
echo ">>> Lanzando los trabajos restantes (con pausas anti-bloqueo) <<<"

# Configuración
GRAIN_BATCHES=("0 1" "32 64" "128 256" "512 1024" "2048 4096")
PARTITIONERS=("auto" "simple" "static" "affinity")

# 1. OPTIMIZACIÓN
for PART in "${PARTITIONERS[@]}"; do
    for GRAINS in "${GRAIN_BATCHES[@]}"; do
        echo "Lanzando Opt $PART..."
        sshpass -f .password ssh a0522100@avignon.lab.inf.uc3m.es \
            "cd rtx && sbatch scripts/remote/sweep_optimization.sh \"$PART\" \"$GRAINS\""
        
        # PAUSA VITAL: 2 segundos para respirar
        sleep 2
    done
done

# 2. GRAIN SWEEP
for PART in "${PARTITIONERS[@]}"; do
    for GRAINS in "${GRAIN_BATCHES[@]}"; do
        echo "Lanzando Grain $PART..."
        sshpass -f .password ssh a0522100@avignon.lab.inf.uc3m.es \
            "cd rtx && sbatch scripts/remote/sweep_grain.sh 56 \"$PART\" \"$GRAINS\""
        
        # PAUSA VITAL
        sleep 2
    done
done

echo ">>> ¡Ahora sí! Revisa squeue."