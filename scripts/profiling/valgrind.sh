echo "Iniciando runs"
for i in $(seq 1 4);
do
    echo "--- Iniciando ejecución ${i} en $(hostname) ---"
    valgrind --tool=cachegrind /workspace/out/build/default/soa/Debug/render-soa /workspace/res/scene_scripts/scene${i}example.txt /workspace/res/config_scripts/config${i}example.txt /workspace/outImagSOA${i}.ppm
done
echo "FIN espero que hayan salido bien"