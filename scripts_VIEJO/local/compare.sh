echo "Iniciando runs"
for i in $(seq 1 4);
do
    echo "--- Iniciando verificación ${i} en $(hostname) ---"
    python3 /workspace/scripts/python/eq_ppm.py /workspace/outImagSOA${i}.ppm /workspace/outImagAOS${i}.ppm 
done
echo "FIN espero que hayan salido bien"