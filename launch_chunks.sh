#!/bin/bash
# launch_chunks.sh - Divide y vence al límite de 10 minutos

echo ">>> Lanzando Enjambre de Trabajos Pequeños <<<"

# 1. ESCALABILIDAD (Chunks de 2 en 2)
# Rango 1-8 (Baseline)
for i in $(seq 1 2 8); do
    END=$((i+1))
    echo "Lanzando Scale $i-$END..."
    make sweep-scale PART=static GRAIN=4096 START=$i END=$END STEP=1
done

# Rango 12-52 (Media) - Step 4
for i in $(seq 12 8 52); do
    # Hacemos chunks de 2 pasos (ej: 12 y 16)
    START=$i
    END=$((i+4))
    echo "Lanzando Scale $START-$END (Step 4)..."
    make sweep-scale PART=static GRAIN=4096 START=$START END=$END STEP=4
done

# Rango 56-120 (Alta) - Step 4
for i in $(seq 56 8 120); do
    START=$i
    END=$((i+4))
    echo "Lanzando Scale $START-$END (Step 4)..."
    make sweep-scale PART=static GRAIN=4096 START=$START END=$END STEP=4
done

# 2. OPTIMIZACIÓN (Por Particionador y mitades de grano)
# Granos: 0 1 32 64 128 256 512 1024 2048 4096 (10 granos)
# Dividimos en lotes de 2 granos para estar seguros
GRAIN_BATCHES=("0 1" "32 64" "128 256" "512 1024" "2048 4096")
PARTITIONERS=("auto" "simple" "static" "affinity")

for PART in "${PARTITIONERS[@]}"; do
    for GRAINS in "${GRAIN_BATCHES[@]}"; do
        echo "Lanzando Opt $PART con granos [$GRAINS]..."
        # Usamos sshpass directo porque make sweep-opt no acepta argumentos extra aun
        sshpass -f .password ssh a0522100@avignon.lab.inf.uc3m.es \
            "cd rtx && sbatch scripts/remote/sweep_optimization.sh \"$PART\" \"$GRAINS\""
    done
done

# 3. GRAIN SWEEP (Similar a optimización pero con hilos fijos 56)
# Usamos los mismos lotes
for PART in "${PARTITIONERS[@]}"; do
    for GRAINS in "${GRAIN_BATCHES[@]}"; do
        echo "Lanzando Grain $PART con granos [$GRAINS]..."
        sshpass -f .password ssh a0522100@avignon.lab.inf.uc3m.es \
            "cd rtx && sbatch scripts/remote/sweep_grain.sh 56 \"$PART\" \"$GRAINS\""
    done
done

echo ">>> ¡Todos los torpedos lanzados! Monitoriza con 'squeue -u a0522100' <<<"