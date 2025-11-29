echo "Iniciando runs"
#for i in $(seq 1 4);
#do
#    echo "--- Iniciando ejecución 5 en $(hostname) ---"
    valgrind ./out/build/default/par/Release/render-par ./res/scene_scripts/scene5.txt ./res/config_scripts/config5.txt ./outImagSOA5.ppm > valgrind-text.out
#done
echo "FIN espero que hayan salido bien"