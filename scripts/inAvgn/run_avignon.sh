#!/bin/bash
#esto es ra correlo desde el frontend de avignon, no para hacer el sbatch
echo "EJECUTANDO EN AVIGNON" 
sbatch res/run/run1.sh
sbatch res/run/run2.sh
sbatch res/run/run3.sh
sbatch res/run/run4.sh