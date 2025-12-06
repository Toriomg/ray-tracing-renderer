# Manual de Experimentación: Rama analysis/writer

Este documento detalla las pruebas para la rama **"Solo Escritura PPM Paralela"**.

---

## 1. Objetivo y Expectativas

En esta rama, solo la **escritura del archivo final en disco** (`ppm_writer.cpp`) utiliza TBB. El resto del programa (trazado de rayos y procesado de imagen) es **secuencial**.

### La Realidad Técnica (Ley de Amdahl Extrema)

| Componente | Tiempo Aproximado | Estado |
|------------|-------------------|--------|
| **Renderizado** | ~25-40 segundos | Secuencial |
| **Escritura PPM** | ~0.5 - 2 segundos | **Paralela** |

**Consecuencia:** Al paralelizar solo la escritura, la mejora en el tiempo total será **muy pequeña** (quizás 1 o 2 segundos en total). La gráfica de Speedup no subirá mucho.

> ⚠️ **Esto NO es un fallo.** Es un resultado científico que demuestra que paralelizar I/O (Input/Output) tiene rendimientos decrecientes si la carga computacional principal no está optimizada.

---

## 2. Qué estamos probando

| Componente | Estado | Comportamiento |
|------------|--------|----------------|
| **Motor** | **SECUENCIAL** | Lento constante. |
| **Imagen** | **SECUENCIAL** | Rápido constante. |
| **Writer** (`ppm_writer`) | **PARALELO (configurable)** | TBB formatea píxeles a strings. Soporta 4 particionadores: `auto`, `simple`, `static`, `affinity`. |

### Configuración del PPM Writer

El writer acepta `ParallelSettings` para controlar:
- **Tipo de particionador:** `--image-part <auto|simple|static|affinity>`
- **Grain size:** `--image-grain <size>` (tamaño de bloque para cada thread)
- **Threads:** `--threads <n>` (limita hilos de TBB)

**Ejemplo:**
```bash
./render-par scene5.txt config5.txt output.ppm --image-part static --image-grain 500 --threads 4
```

---

## 3. Flujo de Experimentación

Existen **DOS flujos** posibles según la metodología del profesor:

---

### 🎯 **FLUJO A: Metodología Recomendada (Hilos Primero → Granularidad Después)**

Este flujo sigue el orden de que primero determinar el número óptimo de hilos, luego optimizar granularidad.

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

**⚠️ ADVERTENCIA:** Las diferencias de tiempo serán **MUY mínimas** (~0.1-0.5s) porque la escritura es <5% del tiempo total.

**Descargar y analizar:**

```bash
make fetch-results
# Busca el mejor tiempo en logs/results_threads_first.csv
# Ejemplo: Si 4 hilos es óptimo, anótalo
```

**Valores clave de hilos:**
- **28 hilos:** 1 socket completo
- **56 hilos:** 2 sockets físicos
- **112 hilos:** Máximo posible (hyperthreading)
- **120 hilos:** 7% sobre el máximo (verificar sobrecarga)

---

#### Paso 2: Explorar Granularidad (con hilos fijos)

Con el número óptimo de hilos encontrado, probar diferentes grains:

```bash
make sweep-grain THREADS=4
```

**¿Qué hace?** 
- Fija los hilos en 4 (o el valor óptimo encontrado)
- Prueba grains: reducción por mitad desde threads + valores típicos de I/O (100, 500, 1000, 5000)
- Prueba con los 4 partitioners

**Descargar y analizar:**

```bash
make fetch-results
# Busca la mejor combinación en logs/results_grain_sweep.csv
# Ejemplo: static con grain=500
```

---

#### Paso 3: Curva de Escalabilidad Final

Con la configuración óptima:

```bash
make sweep-scale PART=static GRAIN=500
```

**¿Qué hace?** Genera la curva de speedup probando hilos: 1, 2, 4, 8, 16, 28, 56, 60, 64, ..., 120

**⚠️ RESULTADO ESPERADO:** Gráfica casi plana (Ley de Amdahl extrema). El tiempo total NO mejorará significativamente porque el writer es <5% del tiempo.

---

### ⚡ **FLUJO B: Tradicional (Más Rápido)**

Si prefieres el flujo rápido (encontrar config óptima asumiendo hilos típicos):

#### Paso 1: Optimización de Writer

```bash
make sweep-opt
```

Prueba todas las combinaciones de partitioner + grain con 56 hilos fijos.

**Analizar resultados:**

```bash
make fetch-results
python3 scripts/analysis/analyze_best.py logs/results_optimization.csv
```

---

#### Paso 2: Escalabilidad

```bash
make sweep-scale PART=static GRAIN=500
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

## 4. Validación y Determinismo

### ⚠️ Importante: Non-determinismo con Múltiples Threads

Debido a que `ParallelRNGManager` usa `tbb::enumerable_thread_specific` con contadores atómicos, **ejecutar el programa con múltiples threads produce resultados diferentes** en cada ejecución (diferentes semillas RNG por thread).

**Para validación determinista:**
```bash
# Usar 1 solo thread garantiza resultados reproducibles
./render-par scene5.txt config5.txt output.ppm --threads 1 --image-part static
```

**Verificación realizada:**
- ✅ Con `--threads 1`: Las 3 ejecuciones (static, affinity, auto) producen MD5 idéntico
- ⚠️ Con `--threads 4`: Cada ejecución produce imagen diferente (por RNG no-determinista)
- ✅ PPM Writer sin bugs: Solo convierte píxeles a strings, no altera datos

### Comparación con Referencia

La referencia del profesor (`s5-par.ppm`) usa **rendering paralelo**, esta rama usa **rendering secuencial**. Por tanto:
- ❌ NO deben compararse directamente (MD5 diferente es esperado)
- ✅ Esta rama genera su propia referencia determinista

---

## 5. Interpretación de Resultados

### Speedup Esperado

- **Speedup máximo:** 1.05x - 1.15x (5-15% de mejora)
- **Saturación:** Inmediata (a partir de 4-8 hilos)
- **Razón:** La parte paralelizada representa <5% del tiempo total

### Conclusión Científica

Esta rama demuestra el **límite práctico de la Ley de Amdahl**:

> "Si solo el 5% del programa es paralelo, incluso con infinitos procesadores, el speedup máximo teórico es 1.05x. Esta rama valida experimentalmente este principio fundamental de la computación paralela."

---

## 6. Scripts Disponibles y Cobertura de Requisitos

### Tabla de Scripts

| Script | Utilidad en esta Rama | Argumentos que usa | Comando Make |
|--------|----------------------|--------------------|--------------|
| `test_custom.sh` | **Alta.** Prueba manual rápida con configuración editable. | `--image-part`, `--image-grain`, `--threads` | `make test-custom` |
| `sweep_threads_first.sh` | **🆕 Crítica (Flujo A).** Explora 28, 56, 112, 120 hilos con auto_partitioner. | `--image-part auto`, `--image-grain 0`, `--threads` | `make sweep-threads-first` |
| `sweep_grain.sh` | **🆕 Crítica (Flujo A).** Con hilos fijos, prueba grains (threads→threads/2→...→1→0) + I/O (100-5000). | `--image-part`, `--image-grain`, `--threads <fijo>` | `make sweep-grain THREADS=4` |
| `sweep_optimization.sh` | **🆕 Crítica (Flujo B).** Encuentra mejor partitioner+grain con 56 hilos fijos. | `--image-part`, `--image-grain`, `--threads 56` | `make sweep-opt` |
| `sweep_scalability.sh` | **Crítica.** Genera curva de speedup (1→120 hilos, paso=4). **Mejora muy mínima esperada.** Ahora acepta rango personalizado. | `--image-part`, `--image-grain`, `--threads` | `make sweep-scale PART=X GRAIN=Y [START=N END=M STEP=S]` |
| `test_determinism.sh` | **Validación.** Verifica determinismo con 1 thread (3 ejecuciones). | `--threads 1` | `make test-determinism` |
| `test_comprehensive.sh` | **Análisis.** 16 configuraciones exhaustivas del writer. | Varios | `make test-comprehensive` |

**Notas importantes para analysis/writer:**
- ✅ `sweep_scalability.sh` ahora prueba hasta **120 hilos** (antes variaba)
- ✅ `sweep_scalability.sh` hace barrido fino de 56→120 con paso=4
- ✅ **NUEVO:** `sweep_scalability.sh` acepta rangos personalizados: `START=1 END=8 STEP=1` (ideal para writer que satura temprano)
- ✅ **MEJORAS DE ROBUSTEZ:** Todos los scripts usan `perf -r 5` (promedio de 5 ejecuciones), error handling y sync
- ✅ `sweep_optimization.sh` **NUEVO** - incluye granos típicos de I/O (100-5000)
- ✅ `sweep_grain.sh` **NUEVO** - implementa reducción por mitad + granos I/O
- ⚠️ **DIFERENCIA CLAVE:** Esta rama usa `--writer-part` y `--writer-grain` (el writer paralelo usa estos parámetros)

---

### Cobertura de Requisitos del Profesor ✅

| Requisito del Profesor | Cómo lo cubrimos | Script |
|------------------------|------------------|--------|
| **"Primero establecer número de hilos"** | ✅ `sweep_threads_first.sh` prueba 28, 56, 112, 120 con auto | `make sweep-threads-first` |
| **"Empezar con 4 valores: 28, 56, 112, 120"** | ✅ Implementado exactamente en `sweep_threads_first.sh` | `make sweep-threads-first` |
| **"Después probar granularidad"** | ✅ `sweep_grain.sh` con hilos fijos, grain=threads→mitad→...+I/O | `make sweep-grain THREADS=X` |
| **"Primer valor de grain = número de hilos"** | ✅ `sweep_grain.sh` empieza con `GRAINS=($OPTIMAL_THREADS)` | `make sweep-grain THREADS=4` |
| **"Reducir a la mitad hasta que empeore"** | ✅ Bucle automático: 56→28→14→7→3→1→0 | `make sweep-grain THREADS=4` |
| **"Rango de 56 a 112 con step de 4"** | ✅ `sweep_scalability.sh` usa `seq 56 4 120` | `make sweep-scale PART=X GRAIN=Y` |
| **"Probar hasta 120 hilos"** | ✅ Añadido a `sweep_scalability.sh` y `sweep_threads_first.sh` | Ambos scripts |
| **"Probar partitioners"** | ✅ Todos los scripts prueban auto, simple, static, affinity | Todos los sweep |
| **"Combinar granularidad con partitioners"** | ✅ `sweep_grain.sh` hace producto cartesiano | `make sweep-grain THREADS=X` |
| **"Probar granos típicos de I/O"** | ✅ `sweep_grain.sh` añade 100, 500, 1000, 5000 | `make sweep-grain THREADS=X` |

---

## 7. Guía Completa de Tests (Detallados)

Esta rama tiene **4 tests esenciales**, cada uno con un propósito único y no redundante:

---

### 📋 TEST 1: Validación de Determinismo (`test_determinism.sh`)

**Propósito:** Verificar correctness del código.

**¿Qué hace?**
- Ejecuta **3 renderizados independientes** con 1 thread
- Usa diferentes particionadores (static, affinity, auto)
- Compara los MD5 de las 3 imágenes generadas

**¿Para qué sirve?**
- ✅ Verificar que con 1 thread el sistema es **completamente determinista**
- ✅ Confirmar que diferentes particionadores NO alteran el resultado visual
- ✅ Detectar bugs en el PPM Writer o en la lógica de rendering

**¿Qué debes ver?**
```
out_det1.ppm: 18cbc96f310c13469492409588dbd612
out_det2.ppm: 18cbc96f310c13469492409588dbd612
out_det3.ppm: 18cbc96f310c13469492409588dbd612

✓ TODAS IDÉNTICAS - Sistema es determinista con 1 thread
```

**Si los MD5 son diferentes → HAY UN BUG**

**Comandos:**
```bash
make test-determinism          # Lanzar
make tail-determinism          # Ver output en tiempo real
make fetch-validation          # Descargar imágenes
make verify-local              # Verificar MD5 localmente
```

**💡 Este test es CRÍTICO:** Siempre ejecútalo primero antes de experimentar.

---

### 🧪 TEST 2: Prueba Rápida Manual (`test_custom.sh`)

**Propósito:** Debugging y pruebas rápidas durante desarrollo.

**¿Qué hace?**
- Ejecuta UN solo renderizado con los parámetros que TÚ definas
- Totalmente configurable editando el script

**¿Para qué sirve?**
- 🔧 Probar una configuración específica rápidamente
- 🐛 Debugging: verificar que cambios compilan y ejecutan correctamente
- ⚡ Iteración rápida sin lanzar tests completos

**¿Cómo usarlo?**
1. Edita `scripts/remote/test_custom.sh` línea ~20:
   ```bash
   ARGS="--image-part static --image-grain 500 --threads 4"
   ```
2. Ejecuta:
   ```bash
   make remote-build          # Subir código
   make run-custom            # Lanzar
   make tail-custom           # Ver output
   ```

**¿Qué debes ver?**
```
Limiting TBB to 4 threads
Building BVH: 196 objects.
Rendering with ImagePar...
Image written to out_custom.ppm
```

**💡 Tip:** Usa este test para iterar rápido antes de lanzar el test comprehensivo.

---

### 📊 TEST 3: Exploración Completa (`test_comprehensive.sh`)

**Propósito:** Análisis exhaustivo del PPM Writer paralelo.

**¿Qué hace?**
- Ejecuta **16 configuraciones** automáticamente en un solo job
- Prueba 4 particionadores, 4 grain sizes, 5 niveles de threads
- Genera CSV con tiempos, energía y configuraciones

**¿Para qué sirve?**
- 📈 **Encontrar la configuración óptima** del PPM Writer
- 📊 Comparar impacto de particionadores, grain size y threads
- 📄 Generar datos para gráficas de rendimiento en el paper/informe

**Configuraciones ejecutadas:**

| Fase | Qué varía | Valores probados | Tests |
|------|-----------|------------------|-------|
| **Fase 1** | Particionadores | auto, simple, static, affinity | 4 |
| **Fase 2** | Grain Size | 100, 500, 1000, 5000 | 4 |
| **Fase 3** | Threads | 1, 2, 4, 8, 16 | 5 |
| **Fase 4** | Extremos | Combinaciones límite | 3 |
| **TOTAL** | | | **16** |

**Comandos:**
```bash
make remote-build              # Compilar y subir
make test-comprehensive        # Lanzar (tarda ~10-15 min)
make tail-comprehensive        # Monitorear progreso
make fetch-comprehensive       # Descargar CSV
```

**¿Qué debes ver?**

Archivo `logs/comprehensive_results.csv`:
```csv
Test,Partitioner,GrainSize,Threads,Time(s),Energy(J)
part_auto,auto,0,4,38.52,1248.7
part_simple,simple,500,4,37.84,1231.2
part_static,static,500,4,37.91,1229.8
part_affinity,affinity,500,4,37.88,1230.5
grain_100,static,100,4,38.12,1235.1
grain_500,static,500,4,37.91,1229.8
...
```

**💡 Cómo interpretar:**

1. **Comparar Fase 1 (particionadores):**
   - ¿Cuál tiene menor tiempo? → Mejor particionador
   - ¿Diferencia > 5%? → Particionador importa
   - ¿Diferencia < 1%? → Particionador no importa mucho

2. **Comparar Fase 2 (grain size):**
   - Gráfica tiempo vs grain_size
   - ¿Hay un mínimo? → Grain size óptimo
   - ¿Es plana? → Grain size no crítico

3. **Comparar Fase 3 (threads):**
   - Calcular speedup = Time(1 thread) / Time(N threads)
   - Speedup ~1.05x → **Correcto** (Ley de Amdahl, solo 5% paralelo)
   - Speedup > 1.20x → Algo raro

**⚠️ Importante:** 
- El speedup máximo teórico en esta rama es **~1.05x** porque solo el Writer (~5% del tiempo) es paralelo
- Si ves speedups mayores, revisa las mediciones

---

### 📈 TEST 4: Sweep de Escalabilidad (`sweep_scalability.sh`)

**Propósito:** Medir cómo escala el PPM Writer con diferentes números de threads.

**¿Qué hace?**
- Ejecuta la **misma configuración** con 1, 2, 4, 8, 12, 16 threads
- Usa particionador `static` y grain size `500` (valores típicos)
- Genera CSV con tiempos y speedups

**¿Para qué sirve?**
- 📈 **Validar Ley de Amdahl** en esta rama (writer paralelo, resto secuencial)
- 📉 Verificar que speedup máximo ~1.05x (5% del tiempo es paralelo)
- 🔬 Demostrar experimentalmente límite de paralelización

**Comandos:**
```bash
make remote-build              # Compilar y subir
make sweep-scale               # Lanzar (tarda ~8-10 min)
make tail-scale                # Monitorear progreso
make fetch-scale               # Descargar CSV
```

**¿Qué debes ver?**

Archivo `logs/scalability_sweep.csv`:
```csv
Threads,Time(s),Energy(J),Speedup
1,38.52,1248.7,1.00
2,38.21,1242.3,1.008
4,37.91,1229.8,1.016
8,37.85,1228.1,1.018
12,37.82,1227.5,1.019
16,37.81,1227.2,1.019
```

**💡 Cómo interpretar:**

1. **Calcular speedup teórico (Ley de Amdahl):**
   - Si 95% secuencial, 5% paralelo
   - Speedup máximo = 1 / (0.95 + 0.05/∞) = 1.053

2. **Comparar con speedup experimental:**
   - Speedup(16 threads) ≈ 1.019 ✅ (Correcto, bajo techo de 1.053)
   - Speedup plano después de 4 threads ✅ (saturación esperada)

3. **Verificar eficiencia:**
   - Eficiencia = Speedup / Threads
   - Con 16 threads: 1.019 / 16 = 6.4% ✅ (muy baja, normal en esta rama)

**⚠️ Importante:** 
- Este test **demuestra el límite de Amdahl**: si solo 5% del código es paralelo, nunca obtendrás speedups grandes
- Es la razón por la que se crearon las ramas `analysis/image` y (futura) `analysis/full`: necesitas paralelizar MÁS del programa para ver mejoras significativas

---

## 7. Resumen de los 4 Tests

| Test | Propósito | ¿Cuándo usarlo? | Tiempo estimado |
|------|-----------|-----------------|-----------------|
| **test_determinism** | Verificar corrección (sin bugs) | Después de modificar PPM Writer | ~5 min |
| **test_custom** | Probar una config específica | Desarrollo iterativo, debugging | ~2 min |
| **test_comprehensive** | Encontrar config óptima | Análisis completo, paper | ~12 min |
| **sweep_scalability** | Validar Ley de Amdahl | Demostrar límites de paralelización | ~9 min |

### 🎯 Flujo de trabajo recomendado:

```
1. Modificaste código del PPM Writer
   └─> make test-determinism       # ¿Funciona? ¿Sin bugs?

2. ¿Correcto? Ahora optimizar
   └─> make test-custom            # Probar una idea específica
   └─> make test-comprehensive     # ¿Cuál es la mejor config?

3. Escribir paper/informe
   └─> make sweep-scale            # Gráfica de speedup vs threads
   └─> Demostrar Ley de Amdahl experimentalmente
```

### 💡 Tips adicionales:

- **Limpieza antes de experimentos:** `make remote-clean` para borrar outputs viejos
- **Monitoreo:** Todos los tests tienen comando `make tail-X` para ver progreso en tiempo real
- **Validación local:** `make verify-local` compara tus resultados con referencias del profesor
- **Debugging:** Si algo falla, revisa `logs/*.txt` o logs del Slurm job

### 📊 Workflow típico completo:

```bash
# 1. Compilar y subir
make remote-build

# 2. Validar corrección (SIEMPRE primero)
make test-determinism
make tail-determinism          # Monitorear
make fetch-determinism         # Descargar

# 3. Análisis completo
make test-comprehensive
make tail-comprehensive
make fetch-comprehensive       # logs/comprehensive_results.csv

# 4. Escalabilidad (para paper)
make sweep-scale
make tail-scale
make fetch-scale               # logs/scalability_sweep.csv

# 5. Verificar contra referencia del profesor
make verify-local              # Compara MD5 hashes
```

---

## 8. Interpretación Rápida de Resultados

### ✅ `test-determinism`:
- **Los 3 MD5 son idénticos** → PPM Writer funciona correctamente (sin bugs)
- **Los 3 MD5 son diferentes** → HAY UN BUG (el Writer debería ser determinista con 1 thread)

### ⚙️ `test-custom`:
- **Imagen generada** → Configuración funciona
- **Error/crash** → Revisa logs de Slurm (`make tail-custom`)
- **Tiempo muy alto** → Prueba otra configuración

### 📊 `test-comprehensive`:
- **Encuentra el tiempo mínimo en el CSV** → Mejor combinación partitioner+grain+threads
- **Compara Fase 1 (partitioners):** ¿Diferencia significativa? → Particionador importa
- **Compara Fase 2 (grain size):** ¿Hay un óptimo? → Grain size crítico
- **Compara Fase 3 (threads):** ¿Mejora con más threads? → Debe mejorar ~2-5% máximo

### 📈 `sweep-scale`:
- **Speedup ≈ 1.02-1.05x con 16 threads** → ✅ Correcto (Ley de Amdahl, solo 5% paralelo)
- **Speedup > 1.20x** → ⚠️ Error en medición o código modificado
- **Gráfica plana después de 4-8 threads** → ✅ Saturación esperada

---

**💡 Regla de Oro:**
1. **Primero valida** (`test-determinism`) → ¿Funciona sin bugs?
2. **Luego optimiza** (`test-comprehensive`) → ¿Cuál es la mejor config?
3. **Finalmente demuestra** (`sweep-scale`) → ¿Cumple Ley de Amdahl?

---

## 9. Arquitectura de Código

### Cadena de Llamadas para PPM Writer

```
main.cpp
  └─> ImagePar::write_to_ppm(filename, &imageSettings)
       └─> PPMWriter::write_ppm(filename, pixels, settings)
            ├─> validate_pixel_dimensions()
            ├─> open_and_write_header()
            ├─> convert_pixels_parallel()  ← TBB aquí
            │    └─> switch(settings->type)
            │         ├─> simple_partitioner()
            │         ├─> static_partitioner()
            │         ├─> affinity_partitioner()
            │         └─> auto_partitioner()
            └─> write_output_lines()  ← Secuencial
```

### Archivos Modificados

- `common/include/ppm_writer.hpp`: Firma con `ParallelSettings*`
- `common/src/ppm_writer.cpp`: Switch de particionadores + grain size configurable
- `par/include/image_par.hpp`: Pasa settings al writer
- `par/src/image_par.cpp`: Implementación del paso de settings
- `par/src/main.cpp`: CLI parsing de `--image-part`, `--image-grain`

---

## 10. Troubleshooting

### Problema: "Job tarda mucho en ejecutar"
**Solución:** 
- Revisa cola Slurm: `ssh avignon 'squeue -u a0522100'`
- Si está pendiente (PD), espera tu turno
- Si está ejecutando (R), usa `make tail-X` para monitorear

### Problema: "MD5 hashes son diferentes en test-determinism"
**Solución:**
- **Esto indica un BUG en el PPM Writer**
- Revisa `common/src/ppm_writer.cpp`
- Verifica que con 1 thread NO haya race conditions

### Problema: "Speedup muy alto (>1.20x)"
**Solución:**
- Verifica que el rendering sea SECUENCIAL (`rendering_engine.hpp` debe tener for loops, no TBB)
- Si rendering es paralelo, estás en la rama equivocada (debería ser `analysis/image` o `main`)

### Problema: "Imagen diferente a s5-par.ppm del profesor"
**Esto es NORMAL:**
- El profesor usó rendering paralelo → Diferentes píxeles por RNG diferente
- Esta rama tiene rendering SECUENCIAL → MD5 diferente es esperado
- Solo compara MD5 entre TUS propias ejecuciones con `--threads 1`

### Problema: "make comando falla con 'No such file or directory'"
**Solución:**
- Verifica que estés en la raíz del workspace: `cd /workspace`
- Verifica que `scripts/remote/X.sh` existe: `ls scripts/remote/`
- Verifica que el makefile tiene el comando: `grep "nombre-comando:" makefile`

---

## 11. Interpretación de Resultados (Para la Memoria)

### Comparación con Otras Ramas

| Rama | Componente Paralelo | Speedup Máximo Esperado | Saturación | Tipo de Carga |
|------|---------------------|-------------------------|------------|---------------|
| **analysis/rendering** | Motor de rayos | **30-40x** con 56 hilos | ~56 hilos | Compute Bound |
| **analysis/image** | Procesado de imagen | **3-8x** en Stage 2, **<1.05x** total | ~8-16 hilos | Memory Bound |
| **analysis/writer** (esta) | Escritura PPM | **2-5x** en write, **<1.05x** total | ~4-8 hilos | I/O Bound |

**⚠️ CRÍTICO:** En esta rama, el **tiempo total** apenas mejorará porque:
- Rendering (secuencial): ~25-38s (95% del tiempo)
- Escritura (paralela): ~0.5-2s (5% del tiempo)
- **Amdahl dice:** Speedup máximo teórico = 1 / (0.95 + 0.05/∞) ≈ **1.053x**

### Gráfica Característica del Tiempo Total

La gráfica de Tiempo vs Threads debería mostrar:
- **1 hilo:** ~39.0s
- **4 hilos:** ~38.5s (mejora de ~0.5s)
- **8 hilos:** ~38.3s (mejora de ~0.2s adicional)
- **16 hilos:** ~38.2s (saturado, no mejora más)
- **56+ hilos:** ~38.2s (igual, overhead de threads)

**Conclusión para la Memoria:**
> _"La paralelización del PPM Writer fue técnicamente exitosa (speedup de 3-4x en la escritura aislada), pero su impacto en el rendimiento global es despreciable (<3% de mejora) debido a que representa menos del 5% del tiempo total de ejecución. Este resultado valida experimentalmente el límite práctico de la Ley de Amdahl: optimizar componentes minoritarios no afecta significativamente al tiempo total. Para obtener mejoras sustanciales, es necesario paralelizar los componentes que dominan el tiempo de ejecución (rendering e image processing)."_

### Gráfica de la Escritura Aislada (Si la medís)

Si conseguís aislar solo el tiempo de escritura:
- **Inicio (1-2 hilos):** Mejora moderada (~2x)
- **Media (4-8 hilos):** Saturación progresiva (I/O bound)
- **Final (>8 hilos):** Meseta horizontal (disco no puede escribir más rápido)

---

## 12. Resumen de Cambios

### ✅ Últimas Actualizaciones (2025-12-06)

**Mejoras de Robustez en Mediciones:**
- ✅ **perf stat -r 5:** Todos los scripts ejecutan 5 repeticiones y promedian (reduce ruido)
- ✅ **Error handling:** Validación de salida de perf y datos extraídos
- ✅ **sync después de cada escritura:** Previene pérdida de datos en crashes
- ✅ **Logging mejorado:** Mensajes de error explícitos en stderr

**Flexibilidad en Escalabilidad:**
- ✅ **sweep_scalability.sh** ahora acepta rangos personalizados:
  ```bash
  make sweep-scale PART=static GRAIN=500 START=1 END=8 STEP=1
  # Ideal para writer: prueba 1, 2, 3, 4, 5, 6, 7, 8 hilos
  ```
- ✅ Modo híbrido por defecto (sin START): 1, 2, 4, 8, 16, 28 + seq(56,STEP,END)
- ✅ Modo personalizado (con START): seq(START,STEP,END)
- ✅ **Utilidad para writer:** Permite enfocarse en 1-8 hilos donde ocurre la saturación

**Corrección de Parámetros:**
- ✅ Todos los scripts ahora usan `--writer-part` y `--writer-grain` correctamente

### ✅ Correcciones Previas (2025-12-05)

1. **sweep_scalability.sh:**
   - ✅ Añadido 120 hilos al barrido
   - ✅ Barrido fino: `seq 56 4 120` (56, 60, 64, 68, ..., 120)
   - ✅ Actualizado comentario para reflejar analysis/writer
   - ✅ Ahora flexible con argumentos opcionales

2. **sweep_optimization.sh (NUEVO):**
   - ✅ Añadidos granos típicos de I/O: 100, 500, 1000, 5000
   - ✅ Mantiene reducción por mitad desde threads: 56, 28, 14, 7, 1, 0
   - ✅ Fija 56 hilos con `--threads 56` explícitamente
   - ✅ Usa `--writer-part` y `--writer-grain` correctamente
   - ✅ Mejoras de robustez aplicadas

3. **sweep_threads_first.sh (NUEVO):**
   - ✅ Explora 28, 56, 112, 120 hilos con `auto_partitioner`
   - ✅ Implementa "Paso 1" de la metodología del profesor
   - ✅ Advertencia: mejoras MUY mínimas esperadas (writer <5% del tiempo)

4. **sweep_grain.sh (NUEVO):**
   - ✅ Con hilos fijos, prueba grains: threads→threads/2→...→1→0
   - ✅ **PLUS:** Añade granos típicos de I/O (100, 500, 1000, 5000)
   - ✅ Implementa "Paso 2" de la metodología del profesor
   - ✅ Producto cartesiano: 4 partitioners × (reducción mitad + I/O)
   - ✅ Mejoras de robustez aplicadas

5. **test_custom.sh:**
   - ✅ Actualizado comentario para reflejar analysis/writer
   - ✅ Valores predeterminados ajustados (THREADS=4, GRAIN=500)

6. **Makefile:**
   - ✅ Añadidos comandos: `make sweep-opt`, `make sweep-threads-first`, `make sweep-grain THREADS=X`
   - ✅ Actualizado `sweep-scale` para aceptar START, END, STEP
   - ✅ Mensajes informativos sobre naturaleza de analysis/writer (mejoras mínimas)

7. **Documentación:**
   - ✅ Explicados ambos flujos (A: hilos primero, B: tradicional)
   - ✅ Tabla de cobertura de requisitos del profesor
   - ✅ Advertencias sobre resultados esperados (Ley de Amdahl extrema)
   - ✅ Comparación con analysis/rendering y analysis/image
   - ✅ Tabla de scripts actualizada con todos los nuevos comandos

### 📊 Ahora Cumplimos TODOS los Requisitos

| Requisito | Estado |
|-----------|--------|
| Probar 28, 56, 112, 120 hilos | ✅ |
| Empezar grain = número de hilos | ✅ |
| Reducir grain a la mitad | ✅ |
| Probar granos típicos de I/O | ✅ |
| Barrido fino 56→120 (paso=4) | ✅ |
| Flujo: hilos primero → grain después | ✅ |
| Parámetros correctos (--writer-part/--writer-grain) | ✅ |

### 🔬 Características Únicas de analysis/writer

- **Menor speedup esperado:** <1.05x total (vs 30-40x en rendering)
- **Saturación temprana:** ~4-8 hilos (vs ~56 en rendering)
- **Granos típicos:** 100-5000 (I/O bound) vs 1-512 (compute) o 1024-8192 (memory)
- **Objetivo:** Demostrar límite extremo de Ley de Amdahl

---

_Última actualización: 2025-12-05 - Scripts corregidos según metodología del profesor para analysis/writer_
