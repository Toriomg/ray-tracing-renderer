import pandas as pd
import matplotlib.pyplot as plt
import sys

def clean_number(x):
    """Arregla números con formato raro (ej: 6.895.67 -> 6895.67)"""
    if isinstance(x, str):
        # Si hay más de un punto, quitamos todos menos el último
        if x.count('.') > 1:
            parts = x.rsplit('.', 1)
            return float(parts[0].replace('.', '') + '.' + parts[1])
    return float(x)

def main():
    print(">>> Generando gráficas...")
    
    # 1. Cargar y limpiar datos
    try:
        df = pd.read_csv('logs/results_scalability.csv')
    except FileNotFoundError:
        print("Error: No encuentro logs/results_scalability.csv")
        return

    # Limpiar columnas numéricas por si acaso
    df['Time(s)'] = df['Time(s)'].apply(clean_number)
    df['Energy(J)'] = df['Energy(J)'].apply(clean_number)

    # 2. Calcular Speedup y Eficiencia
    base_time = df[df['Threads'] == 1]['Time(s)'].values[0]
    df['Speedup'] = base_time / df['Time(s)']
    df['Efficiency'] = df['Speedup'] / df['Threads']

    # --- GRÁFICA 1: TIEMPO vs HILOS ---
    plt.figure(figsize=(10, 6))
    plt.plot(df['Threads'], df['Time(s)'], 'b-o', label='Tiempo Real')
    # Marcar el mínimo
    min_time = df['Time(s)'].min()
    best_thread = df[df['Time(s)'] == min_time]['Threads'].values[0]
    plt.plot(best_thread, min_time, 'r*', markersize=15, label=f'Óptimo ({best_thread} hilos)')
    
    plt.axhline(y=175, color='r', linestyle='--', label='Objetivo (175s)')
    plt.xlabel('Número de Hilos')
    plt.ylabel('Tiempo (segundos)')
    plt.title('Tiempo de Ejecución vs Hilos')
    plt.legend()
    plt.grid(True)
    plt.savefig('logs/grafica_tiempo.png')
    print("-> Guardada: logs/grafica_tiempo.png")

    # --- GRÁFICA 2: SPEEDUP ---
    plt.figure(figsize=(10, 6))
    plt.plot(df['Threads'], df['Speedup'], 'g-o', label='Tu Speedup')
    plt.plot(df['Threads'], df['Threads'], 'k--', alpha=0.3, label='Speedup Ideal')
    plt.xlabel('Número de Hilos')
    plt.ylabel('Speedup (x veces)')
    plt.title('Speedup (Aceleración)')
    plt.legend()
    plt.grid(True)
    plt.ylim(0, df['Speedup'].max() * 1.1)
    plt.savefig('logs/grafica_speedup.png')
    print("-> Guardada: logs/grafica_speedup.png")

    print(f"\nANÁLISIS FINAL:")
    print(f"Mejor Tiempo: {min_time:.4f} s con {best_thread} hilos.")
    print(f"Speedup Máximo: {base_time/min_time:.2f}x")

if __name__ == "__main__":
    main()