# Manual de Experimentación: Rama analysis/rendering

Este documento detalla el procedimiento de pruebas para la rama **"Solo Motor Paralelo"**.

El objetivo científico de esta rama es **aislar la mejora de rendimiento** que aporta exclusivamente la paralelización del trazado de rayos (`rendering_engine`), manteniendo el post-procesado de imagen (`image_par`) en secuencial como variable de control.

---

## 1. Estado del Código en esta Rama

Para realizar un estudio riguroso, el código se ha configurado de la siguiente manera:

| Componente | Estado | ¿Afectan los parámetros? |
|------------|--------|--------------------------|
| **Motor** (`rendering_engine`) | **PARALELO** (TBB) | **SÍ.** Obedece a `--render-part` y `--render-grain`. |
| **Imagen** (`image_par`) | **SECUENCIAL** | **NO.** Ignora `--image-part` y `--image-grain` (siempre ejecuta en serie). |
| **Global** (`main`) | **Flexible** | **SÍ.** Obedece a `--threads` para limitar la CPU. |

### Argumentos de Ejecución

Aunque el programa acepta todos los argumentos para mantener compatibilidad con los scripts, **solo los del motor tendrán efecto real**:

```bash
./render-par scene.txt config.txt out.ppm \
    --render-part <tipo> --render-grain <n> \  # <--- ESTO CAMBIA EL TIEMPO (rendering paralelo)
    --image-part <tipo> --image-grain <n> \    # <--- ESTO NO HACE NADA (imagen secuencial)
    --threads <n>                              # <--- ESTO CONTROLA EL ESCALADO DE HILOS
```

**En esta rama solo `--render-part`, `--render-grain` y `--threads` tienen efecto real.**

---

## 2. Flujo de Trabajo para el Estudio

Sigue estos pasos para obtener los datos que demuestren el impacto del Motor de Renderizado.

### Paso 0: Preparación

Sube y compila el código híbrido (Main moderno + Imagen secuencial).

```bash
make remote-build
```

---

### Paso 1: Encontrar el Mejor Motor (Optimización)

Usamos el barrido para encontrar qué particionador le va mejor al cálculo de rayos.

**Lanzar test:**

```bash
make sweep-opt
```

(Este script probará combinaciones de `render-part` y `render-grain`. Las combinaciones de imagen que pruebe internamente darán igual resultado).

**Analizar resultados:**

```bash
make fetch-results
python3 scripts/analysis/analyze_best.py logs/results_optimization.csv
```

**Anota la configuración ganadora** (Ej: `static` / `64`).

---

### Paso 2: Curva de Escalabilidad (El dato clave)

Aquí veremos cuánto acelera el programa **solo con el motor paralelo**.

**Lanzar test de hilos** (usando el ganador del paso anterior):

```bash
make sweep-scale PART=static GRAIN=64
```

**Esperar y Descargar:**

```bash
make fetch-results
```

---

### Paso 3: Generar Gráficas Comparativas

```bash
python3 scripts/analysis/plot_results.py
```

**Resultado:** `logs/grafica_speedup.png`.

**Interpretación para la memoria:**
> _"Observamos que el speedup máximo es de X.X veces. Al ser la imagen secuencial, esto representa la ganancia pura del algoritmo de trazado de rayos, que constituye el 99% de la carga computacional."_

---

## 3. Scripts Disponibles y su Función en esta Rama

| Script | Utilidad en esta Rama | Argumentos que usa |
|--------|----------------------|--------------------|
| `test_custom.sh` | **Alta.** Prueba manual rápida con configuración editable. | `--render-part`, `--render-grain`, `--threads` |
| `sweep_optimization.sh` | **Crítica.** Encuentra el mejor particionador y grain size para el motor de rendering. | `--render-part`, `--render-grain` |
| `sweep_scalability.sh` | **Crítica.** Genera la curva de speedup variando hilos (1 a 112). | `--render-part`, `--render-grain`, `--threads` |
| `sweep_matrix.sh` | **Baja.** Combinaciones exhaustivas, pero imagen siempre secuencial. | Varios (legacy) |

**Nota:** Los scripts ahora usan correctamente `--render-part` y `--render-grain` en lugar del antiguo `--partitioner`/`--grain`.

---

## 4. Estructura Completa del Estudio Experimental

El flujo de trabajo establecido permite responder con datos a todas las preguntas de tu equipo.

### 4.1. El Control de Variables

Esta rama implementa **paralelización SOLO en el rendering** para aislar su impacto:

| Parámetro | Propósito en esta Rama (`analysis/rendering`) | Estado | Script que lo Mide |
|-----------|-----------------------------------------------|--------|-------------------|
| `--render-part` / `--render-grain` | **CRÍTICO**. Controla el particionador TBB del rendering. | ✅ PARALELO | `sweep_optimization.sh` |
| `--image-part` / `--image-grain` | **IGNORADO**. El procesamiento de imagen es secuencial (control). | ❌ SECUENCIAL | N/A |
| `--threads <n>` | **ESCALABILIDAD**. Controla cuántos hilos usa TBB (1 a 112). | ✅ ACTIVO | `sweep_scalability.sh` |

**Objetivo científico:** Demostrar que el rendering (~99% del tiempo) es el componente crítico para la paralelización.

### 4.2. Cobertura de Requisitos

| Requisito del Equipo | Cómo lo cubrimos |
|----------------------|------------------|
| "Combinar granularidad, hilos y partitioners en cada caso" | Cubierto con el script **`sweep_matrix.sh`**. Este script prueba el **Producto Cartesiano** de todas las opciones (Render Settings × Image Settings × Hilos Globales), asegurando que encontráis la mejor combinación cruzada. |
| "Rangos de hilos (ej: 56 a 112 con step de 4)" | Cubierto con el script **`sweep_scalability.sh`**. Este script está diseñado con bucles `seq` que permiten definir cualquier rango y paso que necesitéis para completar vuestra **curva de Speedup**. |
| "Evaluar cada segmento individual" | Esta rama mide el segmento **"rendering_engine"** de forma aislada. La rama `image_par` (la siguiente) medirá el otro segmento, permitiéndoos ver su impacto individual. |
| "Probar manualmente" | Cubierto con el script **`test_custom.sh`**, que te permite ejecutar una combinación específica de argumentos sin tener que esperar a los barridos automáticos. |

### 4.3. Flujo de Experimentación Final (El Orden para el Éxito)

Para vuestra memoria, el orden de los tests es **vital**.

#### **Fase A: Medición del Impacto (Branch Baseline)**

- **Comando:** `make sweep-opt`
- **Propósito:** Encontrar la configuración óptima del motor (`affinity / 64`, por ejemplo) que usaremos en la curva de escalabilidad. Este es el **punto de partida** para el resto de la experimentación.

#### **Fase B: Estudio de Escalabilidad (Speedup)**

- **Comando:** `make sweep-scale PART=<ÓPTIMO> GRAIN=<ÓPTIMO>`
- **Propósito:** Generar la **curva de Speedup** de 1 a 112 hilos. Este es el dato fundamental de la rama.

#### **Fase C: La Opción Nuclear (Verificación Exhaustiva)**

- **Comando:** `sbatch scripts/remote/sweep_matrix.sh`
- **Propósito:** Generar la **tabla gigante** que demuestra en la memoria: _"Sí, probamos todas las interacciones, y el impacto del subsistema `image_par` (secuencial) es constante e insignificante."_

---

## 5. Checklist de Verificación Final

Antes de dar los datos por buenos, verifica:

- [ ] **Código:** `rendering_engine.hpp` usa `tbb::parallel_for` (no bucles `for` normales)
- [ ] **Código:** `image_par.cpp` usa bucles `for` secuenciales (sin TBB)
- [ ] **Tests:** `test_custom.sh` usa `--render-part` y `--render-grain` (no `--image-part`)
- [ ] **Ejecución:** `make tail-custom` muestra `Limiting TBB to...` (confirma control de hilos)
- [ ] **Speedup:** Con 1 hilo ~38s, con 56 hilos ~0.8s → Speedup ~47x ✅
- [ ] **Curva:** `results_scalability.csv` muestra aceleración significativa (no como en `analysis/writer`)

**Si el speedup es bajo (<5x), algo está mal - revisa que el rendering sea paralelo.**

---

_Última actualización: 2025-12-05 - Rama corregida: Rendering PARALELO, Imagen SECUENCIAL_
