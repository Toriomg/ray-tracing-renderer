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

Existen **DOS flujos** posibles según la metodología del profesor:

---

### 🎯 **FLUJO A: Metodología Recomendada (Hilos Primero → Granularidad Después)**

Este flujo sirve para primero determinar el número óptimo de hilos, luego optimizar granularidad.

#### Paso 0: Preparación

```bash
make remote-build
```

#### Paso 1: Exploración Inicial de Hilos (con auto_partitioner)

Probar con 28, 56, 112 y 120 hilos usando `auto_partitioner` (grain=0):

```bash
make sweep-threads-first
```

**¿Qué hace?** Prueba los 4 valores clave de hilos con TBB decidiendo el grain automáticamente.

**Descargar y analizar:**

```bash
make fetch-results
# Busca el mejor tiempo en logs/results_threads_first.csv
```

**Ejemplo:** Si 56 hilos es óptimo, anótalo.

#### Paso 2: Explorar Granularidad (con hilos fijos)

Con el número óptimo de hilos encontrado, probar diferentes grains:

```bash
make sweep-grain THREADS=56
```

**¿Qué hace?** 
- Fija los hilos en 56
- Prueba grains: 56, 28, 14, 7, 3, 1, 0 (reducción por mitad desde threads)
- Prueba con los 4 partitioners

**Descargar y analizar:**

```bash
make fetch-results
# Busca la mejor combinación en logs/results_grain_sweep.csv
```

#### Paso 3: Curva de Escalabilidad Final

Con la configuración óptima (ej: 56 hilos, static, grain=28):

```bash
make sweep-scale PART=static GRAIN=28
```

**¿Qué hace?** Genera la curva de speedup probando hilos: 1, 2, 4, 8, 16, 28, 56, 60, 64, ..., 120

---

### ⚡ **FLUJO B: Tradicional (Config Óptima → Escalabilidad)**

Si prefieres el flujo rápido (encontrar config óptima asumiendo máximo de hilos):

#### Paso 0: Preparación

```bash
make remote-build
```

#### Paso 1: Encontrar Configuración Óptima

Prueba todas las combinaciones de partitioner + grain con 56 hilos fijos:

```bash
make sweep-opt
```

**Analizar resultados:**

```bash
make fetch-results
python3 scripts/analysis/analyze_best.py logs/results_optimization.csv
```

**Anota la configuración ganadora** (Ej: `static` / `64`).

#### Paso 2: Curva de Escalabilidad

Usa la config ganadora para generar la curva de speedup:

```bash
make sweep-scale PART=static GRAIN=64
```

**Descargar:**

```bash
make fetch-results
```

---

### 📊 **¿Cuál Flujo Usar?**

| Flujo | Ventajas | Cuándo Usarlo |
|-------|----------|---------------|
| **A: Hilos Primero** | Sigue metodología del profesor, más científico | Paper/Memoria académica |
| **B: Tradicional** | Más rápido, menos tests | Exploración rápida |

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

| Script | Utilidad en esta Rama | Argumentos que usa | Comando Make |
|--------|----------------------|--------------------|--------------|
| `test_custom.sh` | **Alta.** Prueba manual rápida con configuración editable. | `--render-part`, `--render-grain`, `--threads` | `make test-custom` |
| `sweep_threads_first.sh` | **🆕 Crítica (Flujo A).** Explora 28, 56, 112, 120 hilos con auto_partitioner. | `--render-part auto`, `--render-grain 0`, `--threads` | `make sweep-threads-first` |
| `sweep_grain.sh` | **🆕 Crítica (Flujo A).** Con hilos fijos, prueba grains (threads→threads/2→...→1→0). | `--render-part`, `--render-grain`, `--threads <fijo>` | `make sweep-grain THREADS=56` |
| `sweep_optimization.sh` | **Crítica (Flujo B).** Encuentra mejor partitioner+grain con 56 hilos fijos. | `--render-part`, `--render-grain`, `--threads 56` | `make sweep-opt` |
| `sweep_scalability.sh` | **Crítica.** Genera curva de speedup con config óptima (1→120 hilos, paso=4). | `--render-part`, `--render-grain`, `--threads` | `make sweep-scale PART=X GRAIN=Y` |
| `sweep_matrix.sh` | **Baja.** Combinaciones exhaustivas (legacy, muy pesado). | Varios | Manual con `sbatch` |

**Notas importantes:**
- ✅ `sweep_scalability.sh` ahora prueba hasta **120 hilos** (antes solo hasta 112)
- ✅ `sweep_scalability.sh` hace barrido fino de 56→120 con paso=4 (56, 60, 64, 68, ..., 120)
- ✅ `sweep_optimization.sh` ahora incluye grains=56 y grains=112 (igual a número de hilos)
- ✅ `sweep_grain.sh` implementa reducción por mitad desde grain=threads como pide el profesor

---

## 4. Estructura Completa del Estudio Experimental

El flujo de trabajo establecido permite responder con datos a todas las preguntas de tu equipo.

### 4.1. El Control de Variables

Esta rama implementa **paralelización SOLO en el rendering** para aislar su impacto:

| Parámetro | Propósito en esta Rama (`analysis/rendering`) | Estado | Script que lo Mide |
|-----------|-----------------------------------------------|--------|-------------------|
| `--render-part` / `--render-grain` | **CRÍTICO**. Controla el particionador TBB del rendering. | ✅ PARALELO | `sweep_optimization.sh`, `sweep_grain.sh` |
| `--image-part` / `--image-grain` | **IGNORADO**. El procesamiento de imagen es secuencial (control). | ❌ SECUENCIAL | N/A |
| `--threads <n>` | **ESCALABILIDAD**. Controla cuántos hilos usa TBB (1 a 120). | ✅ ACTIVO | `sweep_scalability.sh`, `sweep_threads_first.sh` |

**Objetivo científico:** Demostrar que el rendering (~99% del tiempo) es el componente crítico para la paralelización.

### 4.2. Valores Clave de Hilos (Metodología del Profesor)

| Hilos | Significado | ¿Qué Mide? |
|-------|-------------|------------|
| **28** | 1 socket completo (14 cores × 2 threads) | Rendimiento sin saltar entre sockets NUMA |
| **56** | 2 sockets físicos (máximo paralelismo real) | **Punto óptimo esperado** (sin latencia inter-socket crítica) |
| **112** | Máximo posible (2 × 56 con hyperthreading) | Overhead de hyperthreading vs beneficio |
| **120** | 7% sobre el máximo | **Verificación**: ¿empeora por sobrecarga? (debería) |

**Flujo del profesor:**
1. Probar estos 4 valores con `auto_partitioner` (grain=0)
2. Elegir el mejor (típicamente 56 o 112)
3. Con ese número fijo, probar grains: **empezar con grain=threads**, reducir a la mitad

### 4.3. Cobertura de Requisitos del Profesor ✅

| Requisito del Profesor | Cómo lo cubrimos | Script |
|------------------------|------------------|--------|
| **"Primero establecer número de hilos"** | ✅ `sweep_threads_first.sh` prueba 28, 56, 112, 120 con auto | `make sweep-threads-first` |
| **"Empezar con 4 valores: 28, 56, 112, 120"** | ✅ Implementado exactamente en `sweep_threads_first.sh` | `make sweep-threads-first` |
| **"Después probar granularidad"** | ✅ `sweep_grain.sh` con hilos fijos, grain=threads→mitad→... | `make sweep-grain THREADS=X` |
| **"Primer valor de grain = número de hilos"** | ✅ `sweep_grain.sh` empieza con `GRAINS=($OPTIMAL_THREADS)` | `make sweep-grain THREADS=56` |
| **"Reducir a la mitad hasta que empeore"** | ✅ Bucle automático: 56→28→14→7→3→1→0 | `make sweep-grain THREADS=56` |
| **"Rango de 56 a 112 con step de 4"** | ✅ `sweep_scalability.sh` usa `seq 56 4 120` | `make sweep-scale PART=X GRAIN=Y` |
| **"Probar hasta 120 hilos"** | ✅ Añadido a `sweep_scalability.sh` y `sweep_threads_first.sh` | Ambos scripts |
| **"Probar partitioners"** | ✅ Todos los scripts prueban auto, simple, static, affinity | Todos los sweep |
| **"Combinar granularidad con partitioners"** | ✅ `sweep_grain.sh` hace producto cartesiano | `make sweep-grain THREADS=X` |
| **"Probar manualmente valores cercanos"** | ✅ `test_custom.sh` permite editar cualquier parámetro | `make test-custom` |

### 4.4. Flujo de Experimentación Recomendado (Metodología del Profesor)

Para vuestra memoria, sigue **este orden científico**:

#### **Fase 1: Exploración Inicial de Hilos** 🎯

```bash
make remote-build
make sweep-threads-first
make fetch-results
```

**Resultado:** `logs/results_threads_first.csv` con tiempos de 28, 56, 112, 120 hilos.

**Decisión:** Identifica el número óptimo (ejemplo: 56 hilos).

#### **Fase 2: Optimización de Granularidad** 🔬

```bash
make sweep-grain THREADS=56
make fetch-results
```

**Resultado:** `logs/results_grain_sweep.csv` con todas las combinaciones partitioner × grain.

**Decisión:** Identifica la mejor config (ejemplo: static, grain=28).

#### **Fase 3: Curva de Escalabilidad Final** 📈

```bash
make sweep-scale PART=static GRAIN=28
make fetch-results
```

**Resultado:** `logs/results_scalability.csv` con speedup de 1→120 hilos (paso=4).

**Para la Memoria:** Gráfica de speedup demostrando Ley de Amdahl.

#### **Fase 4 (Opcional): Verificación Exhaustiva** 🚀

```bash
sbatch scripts/remote/sweep_matrix.sh
```

**Propósito:** Tabla completa para demostrar: _"Probamos todas las interacciones posibles"_.

---

## 5. Checklist de Verificación Final

Antes de dar los datos por buenos, verifica:

- [ ] **Código:** `rendering_engine.hpp` usa `tbb::parallel_for` (no bucles `for` normales)
- [ ] **Código:** `image_par.cpp` usa bucles `for` secuenciales (sin TBB)
- [ ] **Tests:** `test_custom.sh` usa `--render-part` y `--render-grain` (no `--image-part`)
- [ ] **Ejecución:** `make tail-custom` muestra `Limiting TBB to...` (confirma control de hilos)
- [ ] **Speedup:** Con 1 hilo ~38s, con 56 hilos ~0.8s → Speedup ~47x ✅
- [ ] **Curva:** `results_scalability.csv` muestra aceleración significativa (no como en `analysis/writer`)
- [ ] **Scripts:** `sweep_scalability.sh` prueba hasta 120 hilos (no solo 112) ✅
- [ ] **Scripts:** `sweep_grain.sh` empieza con grain=threads (56, 112, etc.) ✅
- [ ] **Scripts:** Barrido fino 56→120 con paso=4 en `sweep_scalability.sh` ✅

**Si el speedup es bajo (<5x), algo está mal - revisa que el rendering sea paralelo.**

---

## 6. Resumen de Cambios (2025-12-05)

### ✅ Correcciones Implementadas

1. **sweep_scalability.sh:**
   - ✅ Añadido 120 hilos al barrido
   - ✅ Barrido fino: `seq 56 4 120` (56, 60, 64, 68, ..., 120)

2. **sweep_optimization.sh:**
   - ✅ Añadidos grains=56 y grains=112 (igual a número de hilos)
   - ✅ Fija 56 hilos con `--threads 56` explícitamente

3. **sweep_threads_first.sh (NUEVO):**
   - ✅ Explora 28, 56, 112, 120 hilos con `auto_partitioner`
   - ✅ Implementa "Paso 1" de la metodología del profesor

4. **sweep_grain.sh (NUEVO):**
   - ✅ Con hilos fijos, prueba grains: threads→threads/2→...→1→0
   - ✅ Implementa "Paso 2" de la metodología del profesor
   - ✅ Producto cartesiano: 4 partitioners × N grains

5. **Makefile:**
   - ✅ Añadidos comandos: `make sweep-threads-first`, `make sweep-grain THREADS=X`

6. **Documentación:**
   - ✅ Explicados ambos flujos (A: hilos primero, B: tradicional)
   - ✅ Tabla de cobertura de requisitos del profesor
   - ✅ Flujo recomendado en 4 fases claramente definido

### 📊 Ahora Cumplimos TODOS los Requisitos

| Requisito | Estado |
|-----------|--------|
| Probar 28, 56, 112, 120 hilos | ✅ |
| Empezar grain = número de hilos | ✅ |
| Reducir grain a la mitad | ✅ |
| Barrido fino 56→120 (paso=4) | ✅ |
| Flujo: hilos primero → grain después | ✅ |

---

_Última actualización: 2025-12-05 - Scripts corregidos según metodología del profesor_
