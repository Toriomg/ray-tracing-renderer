if [ -z "$3" ]; then
    echo "Error: No se ha especificado un nombre de usuario."
    echo "Uso: ./cgann.sh <PID> <i>"
    exit 1
fi

PID=$1
IDX=$2

cg_annotate cachegrind.out.${PID} --auto=yes > instr${IDX}.txt