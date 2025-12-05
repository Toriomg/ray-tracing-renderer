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

### Paso 1: Optimización (sweep_optimization.sh)

Aunque el render es secuencial, lanzamos el barrido.

**¿Qué pasará?**
- Cambiar `--render-part` no afectará al tiempo.

**¿Por qué hacerlo?**
- Para demostrar con datos que el motor es, efectivamente, indiferente a los parámetros TBB en esta rama.

```bash
make sweep-opt
```

---

### Paso 2: Escalabilidad (sweep_scalability.sh)

Lanzamos el barrido de hilos.

**Comando:**
```bash
# Usad cualquier configuración, da igual
make sweep-scale PART=auto GRAIN=0
```

**Gráfica Esperada:**
- Una línea **casi plana** que baja ligeramente al principio (cuando pasamos de 1 a 4 hilos) y luego se estanca.
- Esa pequeña bajada es la ganancia del Writer.

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

## 6. Guía Completa de Tests

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

_Última actualización: 2025-12-06 - Rama analysis/writer - 4 tests esenciales_
