echo "Iniciando runs"
for i in $(seq 3 4);
do
    echo "--- Iniciando ejecución ${i} en $(hostname) ---"
    time /workspace/out/build/default/soa/Release/render-soa /workspace/res/scene_scripts/scene${i}example.txt /workspace/res/config_scripts/config${i}example.txt /workspace/outImagSOA${i}.ppm
    echo "--- Iniciando verificación ${i} en $(hostname) ---"
    python3 /workspace/scripts/python/eq_ppm.py /workspace/res/result/s${i}example.ppm /workspace/outImagSOA${i}.ppm 
done
echo "FIN espero que hayan salido bien"