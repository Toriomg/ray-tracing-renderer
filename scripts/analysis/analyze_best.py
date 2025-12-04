import csv
import sys

def main():
    if len(sys.argv) < 2:
        print("Uso: python3 analyze_best.py <archivo.csv>")
        sys.exit(1)

    filename = sys.argv[1]
    best_time = float('inf')
    best_config = None

    try:
        with open(filename, 'r') as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                try:
                    # El CSV tiene cabeceras: Partitioner,GrainSize,Time(s),Energy(J)
                    time_val = float(row['Time(s)'])
                    
                    if time_val < best_time:
                        best_time = time_val
                        best_config = row
                except ValueError:
                    continue # Saltar líneas erróneas si las hay

        if best_config:
            print(f"Mejor Configuración Encontrada:")
            print(f"  -> Tiempo: {best_time} s")
            print(f"  -> Partitioner: {best_config['Partitioner']}")
            print(f"  -> GrainSize: {best_config['GrainSize']}")
            print("-" * 30)
            # Imprimimos formato amigable para bash
            print(f"export BEST_PART={best_config['Partitioner']}")
            print(f"export BEST_GRAIN={best_config['GrainSize']}")
        else:
            print("No se encontraron datos válidos.")

    except FileNotFoundError:
        print(f"Error: El archivo {filename} no existe.")

if __name__ == "__main__":
    main()