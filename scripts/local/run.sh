echo "Iniciando runs"
for i in $(seq 2 4);
do
    echo "--- Iniciando ejecución ${i} en soa $(hostname) ---"
    time /workspace/out/build/default/aos/Release/render-aos /workspace/res/scene_scripts/scene${i}example.txt /workspace/res/config_scripts/config${i}example.txt /workspace/outImagSOA${i}.ppm
    echo "--- Iniciando ejecución ${i} en aos $(hostname) ---"
    time /workspace/out/build/default/aos/Release/render-aos /workspace/res/scene_scripts/scene${i}example.txt /workspace/res/config_scripts/config${i}example.txt /workspace/outImagAOS${i}.ppm
    #echo "--- Iniciando verificación ${i} en $(hostname) ---"
    python3 /workspace/scripts/python/eq_ppm.py /workspace/res/result/s${i}example.ppm /workspace/outImagSOA${i}.ppm 
done
echo "FIN espero que hayan salido bien"