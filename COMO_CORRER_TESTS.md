# 🧪 Guía Completa: Cómo Realizar Tests de Rendimiento

Esta guía explica **paso a paso** cómo ejecutar tests de rendimiento en el cluster Avignon para medir el impacto de la paralelización en diferentes componentes del ray tracer.

> **Última actualización:** 2025-12-06 - Añadidas mejoras de robustez y flexibilidad en escalabilidad

---

## 🆕 Mejoras Recientes (2025-12-06)

### Robustez en Mediciones
- ✅ **perf stat -r 5**: Todos los scripts ejecutan 5 repeticiones y promedian automáticamente
- ✅ **Error handling**: Validación de datos y mensajes de error explícitos
- ✅ **sync automático**: Previene pérdida de datos en crashes

### Flexibilidad en Escalabilidad
- ✅ **sweep-scale** ahora acepta rangos personalizados:
  ```bash
  make sweep-scale PART=static GRAIN=64 START=56 END=112 STEP=2
  # Ejecuta: 56, 58, 60, ..., 112
  ```
- ✅ Sin START: modo híbrido (1,2,4,8,16,28 + seq(56,step,end))
- ✅ Con START: modo personalizado (seq(start,step,end))

**Casos de uso:**
- `START=1 END=8 STEP=1` → Ideal para writer (satura temprano)
- `START=56 END=120 STEP=4` → Barrido fino en zona alta
- Sin START → Curva completa con puntos clave

---

## 📚 Tabla de Contenidos

1. [Conceptos Fundamentales](#-1-conceptos-fundamentales)
2. [Configuración Inicial](#-2-configuración-inicial)
3. [Las Tres Ramas de Análisis](#-3-las-tres-ramas-de-análisis)
4. [Flujo de Trabajo Recomendado](#-4-flujo-de-trabajo-recomendado)
5. [Comandos por Categoría](#-5-comandos-por-categoría)
6. [Metodología del Profesor](#-6-metodología-del-profesor-paso-a-paso)
7. [Interpretación de Resultados](#-7-interpretación-de-resultados)
8. [Troubleshooting](#-8-troubleshooting)

---

## 🎯 1. Conceptos Fundamentales

### ¿Qué estamos midiendo?

El proyecto evalúa el **impacto de paralelizar diferentes componentes** del ray tracer usando **Intel TBB** (Threading Building Blocks):

1. **Rendering Engine** (trazado de rayos): ~25-40 segundos (95-99% del tiempo total)
2. **Image Processing** (gamma + conversión): ~0.1-0.2 segundos (~1% del tiempo)
3. **PPM Writer** (escritura a disco): ~0.5-2 segundos (~5% del tiempo)

### Parámetros de Paralelización

Cada componente paralelo se puede configurar con 3 parámetros:

| Parámetro | Descripción | Valores |
|-----------|-------------|---------|
| **Partitioner** | Estrategia de división de trabajo en chunks | `auto`, `simple`, `static`, `affinity` |
| **Grain Size** | Tamaño mínimo de cada chunk (unidades de trabajo) | `0` (auto), `1`, `32`, `64`, `128`, `256`, `512`, `1024`, ... |
| **Threads** | Número de hilos de ejecución simultáneos | `1`, `2`, `4`, `8`, `16`, `28`, `56`, `112`, `120` |

### Valores Clave de Hilos (Arquitectura del Cluster)

| Hilos | Significado | Cuándo Usar |
|-------|-------------|-------------|
| **1** | Secuencial (baseline) | Referencia para calcular speedup |
| **28** | 1 socket completo (14 cores × 2 threads) | Medir sin latencia entre sockets NUMA |
| **56** | 2 sockets físicos (máximo paralelismo real) | **Punto óptimo esperado** |
| **112** | Máximo con hyperthreading (2 × 56) | Medir overhead de HT |
| **120** | 7% sobre el máximo | Verificar degradación por sobrecarga |

---

## ⚙️ 2. Configuración Inicial

### Requisitos Previos

1. **Acceso a Avignon:** Asegúrate de tener acceso SSH al cluster
2. **Archivo `env.sh`:** Configura tus credenciales (ver README.md)
3. **Archivo `.password`:** Tu contraseña de Avignon (ver README.md)

### Primera Compilación

```bash
# 1. Asegúrate de estar en la raíz del proyecto
cd /workspace

# 2. Sube el código y compila (primera vez)
make remote-build
```

**Esto tarda ~2-3 minutos.** Verás mensajes de CMake compilando el proyecto.

---

## 🌳 3. Las Tres Ramas de Análisis

El proyecto usa **ramas Git separadas** para aislar cada componente paralelo. Esto permite medir el impacto individual de cada optimización.

### 📊 Tabla Comparativa de Ramas

| Rama | Componente Paralelo | % Tiempo | Speedup Esperado | Saturación | Tipo de Carga |
|------|---------------------|----------|------------------|------------|---------------|
| **analysis/rendering** | Motor de rayos | ~99% | **30-40x** (total) | ~56 hilos | Compute Bound |
| **analysis/image** | Procesado de imagen | ~1% | **<1.05x** (total) | ~8-16 hilos | Memory Bound |
| **analysis/writer** | Escritura PPM | ~5% | **<1.05x** (total) | ~4-8 hilos | I/O Bound |

### 🎨 analysis/rendering (Motor Paralelo)

**Objetivo:** Medir el impacto de paralelizar el trazado de rayos.

**Configuración:**
- ✅ Rendering: **PARALELO** (TBB)
- ❌ Image Processing: **SECUENCIAL**
- ❌ PPM Writer: **SECUENCIAL**

**Parámetros que funcionan:**
- `--render-part <partitioner>`
- `--render-grain <size>`
- `--threads <n>`

**Granos típicos:** 1, 32, 64, 128, 256, 512

**Resultado esperado:**
- Speedup: **30-40x** con 56 hilos
- Tiempo: De ~38s (1 hilo) a ~0.8s (56 hilos)
- Gráfica: Crecimiento casi lineal hasta 56 hilos

**Documentación detallada:** `GUIA_RAMA_RENDERING.md`

---

### 🖼️ analysis/image (Procesado Paralelo)

**Objetivo:** Demostrar la **Ley de Amdahl** - paralelizar <1% del código no mejora el tiempo total.

**Configuración:**
- ❌ Rendering: **SECUENCIAL**
- ✅ Image Processing: **PARALELO** (TBB)
- ❌ PPM Writer: **SECUENCIAL**

**Arquitectura especial:**
- **Stage 1:** Rendering secuencial → RawImage buffer (~25s)
- **Stage 2:** Procesado paralelo → Conversión uint8 + gamma (~0.1s)

**Parámetros que funcionan:**
- `--image-part <partitioner>`
- `--image-grain <size>`
- `--threads <n>`

**Granos típicos:** 1024, 2048, 4096, 8192 (más grandes porque la tarea es ligera)

**Resultado esperado:**
- Speedup Stage 2 (aislado): **5-8x**
- Speedup total: **<1.05x** (tiempo casi constante ~25.0-25.5s)
- Gráfica: Casi plana (demostración de Amdahl)

**Documentación detallada:** `GUIA_RAMA_IMAGE.md`

---

### 💾 analysis/writer (Writer Paralelo)

**Objetivo:** Demostrar el **límite extremo de Amdahl** - paralelizar <5% del código.

**Configuración:**
- ❌ Rendering: **SECUENCIAL**
- ❌ Image Processing: **SECUENCIAL**
- ✅ PPM Writer: **PARALELO** (TBB)

**Parámetros que funcionan:**
- `--image-part <partitioner>` (el writer usa estos por implementación)
- `--image-grain <size>`
- `--threads <n>`

**Granos típicos:** 100, 500, 1000, 5000 (I/O bound)

**Resultado esperado:**
- Speedup writer (aislado): **3-5x**
- Speedup total: **<1.05x** (de ~39s a ~38s)
- Gráfica: Casi plana, saturación muy temprana (~4 hilos)

**Documentación detallada:** `GUIA_RAMA_WRITER.md`

---

## 🚀 4. Flujo de Trabajo Recomendado

### 🎯 Flujo A: Metodología del Profesor (Científico)

Este flujo sigue el orden recomendado: **hilos primero → granularidad después**.

```bash
# PASO 0: Preparación
git checkout analysis/rendering  # O la rama que quieras analizar
make remote-build                # Compilar (espera 2-3 min)

# PASO 1: Explorar número óptimo de hilos (28, 56, 112, 120)
make sweep-threads-first
# Espera ~5-8 min, luego:
make fetch-results
# Analiza: logs/results_threads_first.csv
# Ejemplo: "56 hilos da el mejor tiempo"

# PASO 2: Optimizar granularidad (con hilos fijos)
make sweep-grain THREADS=56
# Espera ~10-15 min, luego:
make fetch-results
# Analiza: logs/results_grain_sweep.csv
# Ejemplo: "static con grain=64 es óptimo"

# PASO 3: Curva de escalabilidad (para gráfica de speedup)
make sweep-scale PART=static GRAIN=64
# Espera ~15-20 min, luego:
make fetch-results
# Resultado: logs/results_scalability.csv
# Genera gráfica de speedup vs threads
```

**¿Cuándo usar este flujo?**
- Para la memoria/paper académico
- Cuando necesitas justificar todas las decisiones
- Cuando no conoces el comportamiento del sistema

---

### ⚡ Flujo B: Tradicional (Rápido)

Si ya tienes intuición o quieres explorar rápido:

```bash
# PASO 1: Compilar
git checkout analysis/rendering
make remote-build

# PASO 2: Encontrar configuración óptima
make sweep-opt
# Espera ~10-15 min
make fetch-results
# Analiza: logs/results_optimization.csv

# PASO 3: Generar curva de speedup con config óptima
make sweep-scale PART=static GRAIN=64
make fetch-results
# Resultado: logs/results_scalability.csv
```

**¿Cuándo usar este flujo?**
- Exploración rápida
- Ya conoces el rango de valores óptimos
- Quieres iterar rápidamente

---

### 🔧 Flujo C: Manual (Debugging/Exploración Libre)

Para probar configuraciones específicas rápidamente:

```bash
# 1. Editar configuración manualmente
nano scripts/remote/test_custom.sh
# Cambiar: PARTITIONER, GRAIN_SIZE, THREADS

# 2. Ejecutar
make test-custom

# 3. Ver resultado en tiempo real
make tail-custom

# 4. Repetir con diferentes valores
```

**¿Cuándo usar este flujo?**
- Debugging (verificar que algo funciona)
- Probar una corazonada específica
- Validar antes de lanzar tests largos

---

## 📋 5. Comandos por Categoría

### 🔨 Compilación y Despliegue

| Comando | Descripción | Tiempo estimado |
|---------|-------------|-----------------|
| `make remote-build` | Sube código y compila en Avignon | ~2-3 min |
| `make deploy` | Solo sube código (sin compilar) | ~10-20 seg |

---

### 🧪 Tests Manuales (Flujo C)

| Comando | Descripción | Archivo a editar |
|---------|-------------|------------------|
| `make test-custom` | Ejecuta test con config personalizada | `scripts/remote/test_custom.sh` |
| `make tail-custom` | Ver log en tiempo real | - |

**Cómo usar:**
1. Edita `scripts/remote/test_custom.sh` con los valores que quieras
2. `make test-custom` (lanza el job)
3. `make tail-custom` (monitorea)

---

### 🔬 Tests Automáticos (Flujos A y B)

#### Flujo A (Metodología del Profesor)

| Comando | Qué hace | Tiempo | Archivo resultado |
|---------|----------|--------|-------------------|
| `make sweep-threads-first` | Prueba 28, 56, 112, 120 hilos con auto | ~5-8 min | `results_threads_first.csv` |
| `make sweep-grain THREADS=X` | Prueba grains con X hilos fijos | ~10-15 min | `results_grain_sweep.csv` |
| `make sweep-scale PART=X GRAIN=Y [START=N END=M STEP=S]` | Curva speedup con config óptima (flexible) | ~15-20 min | `results_scalability.csv` |

**Ejemplo con rango personalizado:**
```bash
make sweep-scale PART=static GRAIN=64 START=56 END=112 STEP=2
```

#### Flujo B (Tradicional)

| Comando | Qué hace | Tiempo | Archivo resultado |
|---------|----------|--------|-------------------|
| `make sweep-opt` | Prueba todas las configs (part×grain) | ~10-15 min | `results_optimization.csv` |
| `make sweep-scale PART=X GRAIN=Y [START=N END=M STEP=S]` | Curva speedup con config óptima (flexible) | ~15-20 min | `results_scalability.csv` |

---

### 📥 Descarga de Resultados

| Comando | Descripción |
|---------|-------------|
| `make fetch-results` | Descarga todos los CSVs de `logs/` |
| `make fetch-ppm` | Descarga las imágenes `.ppm` generadas |

---

### 🔍 Validación (Solo analysis/writer)

| Comando | Descripción |
|---------|-------------|
| `make test-determinism` | Verifica que con 1 thread el resultado es determinista |
| `make test-comprehensive` | 16 configuraciones exhaustivas del writer |

---

## 🎓 6. Metodología del Profesor (Paso a Paso)

### Conceptos Clave

El profesor recomienda un flujo específico basado en principios científicos:

1. **Primero: Número de hilos** (lo más importante)
   - Probar 4 valores clave: 28, 56, 112, 120
   - Usar `auto_partitioner` (grain=0) para que TBB decida
   - Objetivo: Encontrar el número óptimo de hilos

2. **Segundo: Granularidad** (con hilos fijos)
   - Empezar con `grain = número de hilos` (ej: grain=56 si threads=56)
   - Reducir a la mitad hasta que empeore: 56→28→14→7→3→1→0
   - Probar los 4 partitioners con cada grain

3. **Tercero: Curva de escalabilidad** (con config óptima)
   - Barrido fino de 56 a 120 (de 4 en 4): 56, 60, 64, 68, ..., 120
   - Usar la mejor combinación encontrada
   - Generar gráfica de speedup para la memoria

---

### Ejemplo Completo: analysis/rendering

#### Paso 1: Explorar Hilos

```bash
git checkout analysis/rendering
make remote-build
make sweep-threads-first
```

**¿Qué hace?**
- Ejecuta con auto_partitioner (grain=0)
- Prueba: 28, 56, 112, 120 hilos
- Genera: `logs/results_threads_first.csv`

**Analizar resultado:**
```bash
make fetch-results
cat logs/results_threads_first.csv
```

**Ejemplo de resultado:**
```csv
Threads,Time(s),Energy(J)
28,1.245,42.3
56,0.812,38.1  ← MEJOR
112,0.835,39.2
120,0.841,39.8
```

**Decisión:** 56 hilos es óptimo.

---

#### Paso 2: Optimizar Granularidad

```bash
make sweep-grain THREADS=56
```

**¿Qué hace?**
- Fija hilos=56
- Prueba grains: 56, 28, 14, 7, 3, 1, 0, 32, 64, 128, 256, 512, 1024
- Prueba los 4 partitioners con cada grain
- Genera: `logs/results_grain_sweep.csv`

**Analizar resultado:**
```bash
make fetch-results
# Buscar línea con menor tiempo
grep "static" logs/results_grain_sweep.csv | sort -t',' -k3 -n | head -5
```

**Ejemplo de resultado:**
```csv
Partitioner,GrainSize,Time(s),Energy(J)
static,64,0.798,37.8  ← MEJOR
static,32,0.812,38.1
affinity,64,0.815,38.3
...
```

**Decisión:** static con grain=64 es óptimo.

---

#### Paso 3: Curva de Escalabilidad

```bash
make sweep-scale PART=static GRAIN=64
```

**¿Qué hace?**
- Usa static + grain=64 (config óptima)
- Prueba hilos: 1, 2, 4, 8, 16, 28, 56, 60, 64, 68, ..., 120
- Genera: `logs/results_scalability.csv`

**Generar gráfica:**
```bash
make fetch-results
python3 scripts/analysis/plot_speedup.py logs/results_scalability.csv
```

**Resultado:** Gráfica `speedup_analysis_rendering.png` mostrando aceleración de 1x a ~40x.

---

### Valores Específicos por Rama

| Rama | Hilos típicos | Granos típicos | Partitioner común |
|------|---------------|----------------|-------------------|
| **analysis/rendering** | 56 | 1-512 | static, affinity |
| **analysis/image** | 56 (pero da igual) | 1024-8192 | auto, static |
| **analysis/writer** | 4-8 (satura rápido) | 100-5000 | static |

---

## 📊 7. Interpretación de Resultados

### Cómo Calcular Speedup

```
Speedup = Tiempo(1 hilo) / Tiempo(N hilos)
```

**Ejemplo:**
- Tiempo con 1 hilo: 38.5s
- Tiempo con 56 hilos: 0.98s
- Speedup = 38.5 / 0.98 ≈ **39.3x**

### Resultados Esperados por Rama

#### ✅ analysis/rendering (ALTA MEJORA)

```csv
Threads,Time(s),Speedup
1,38.52,1.00
2,19.31,1.99
4,9.71,3.97
8,4.92,7.83
16,2.51,15.35
28,1.45,26.56
56,0.98,39.30  ← EXCELENTE
112,0.96,40.12
120,0.97,39.71
```

**Gráfica:** Crecimiento casi lineal hasta 56 hilos, luego meseta.

**Conclusión para memoria:**
> "El rendering paralelo demostró un speedup de 39.3x con 56 hilos, acercándose al límite teórico de 56x. La eficiencia del 70% valida que el rendering es compute-bound y se beneficia significativamente de la paralelización."

---

#### ⚠️ analysis/image (BAJA MEJORA - Ley de Amdahl)

```csv
Threads,Time(s),Speedup
1,25.52,1.00
2,25.48,1.002
4,25.44,1.003
8,25.41,1.004
28,25.35,1.007
56,25.32,1.008  ← Mejora despreciable
112,25.31,1.008
120,25.32,1.008
```

**Gráfica:** Línea casi plana.

**Conclusión para memoria:**
> "La paralelización del procesado de imagen resultó en un speedup de solo 1.008x (0.8% de mejora) con 56 hilos. Este resultado valida la Ley de Amdahl: optimizar un componente que representa <1% del tiempo total no afecta significativamente al rendimiento global."

---

#### ⚠️ analysis/writer (BAJA MEJORA - Ley de Amdahl Extrema)

```csv
Threads,Time(s),Speedup
1,39.12,1.00
2,38.95,1.004
4,38.62,1.013  ← Saturación
8,38.55,1.015
16,38.54,1.015
56,38.53,1.015
```

**Gráfica:** Línea casi plana con saturación muy temprana (~4 hilos).

**Conclusión para memoria:**
> "El writer paralelo alcanzó un speedup máximo de 1.015x (~1.5% de mejora). La saturación temprana (4 hilos) indica que el cuello de botella es I/O (escritura a disco), no CPU. Este resultado demuestra el límite práctico de la Ley de Amdahl: paralelizar <5% del código produce mejoras despreciables."

---

### Comparación Final (Para el Paper)

| Rama | Componente | % Tiempo | Speedup | Tipo | Conclusión |
|------|------------|----------|---------|------|------------|
| **rendering** | Trazado de rayos | 99% | **39x** | Compute | ✅ **Alta mejora** |
| **image** | Post-procesado | 1% | **1.01x** | Memory | ⚠️ Despreciable (Amdahl) |
| **writer** | Escritura PPM | 5% | **1.01x** | I/O | ⚠️ Despreciable (Amdahl) |

**Lección principal:**
> "Para obtener mejoras significativas con paralelización, es necesario optimizar los componentes que dominan el tiempo de ejecución. Paralelizar componentes minoritarios (<5%) produce speedups despreciables, validando experimentalmente la Ley de Amdahl."

---

## 🚨 8. Troubleshooting

### Problema: "Job tarda mucho en ejecutar"

**Diagnóstico:**
```bash
ssh avignon.lab.inf.uc3m.es
squeue -u a052XXXX  # Reemplaza con tu usuario
```

**Estados posibles:**
- `PD` (Pending): En cola, espera tu turno
- `R` (Running): Ejecutando
- `CG` (Completing): Finalizando

**Solución:** Si está `PD`, espera. Si lleva >30 min en `R`, puede haber un problema.

---

### Problema: "MD5 hashes diferentes en test-determinism"

**Causa:** Bug en el código (race condition o non-determinismo).

**Solución:**
1. Revisa que con `--threads 1` el código sea completamente determinista
2. Verifica que no haya variables globales compartidas sin mutex
3. En `analysis/writer`: Esto es NORMAL con múltiples threads (RNG diferente)

---

### Problema: "Speedup muy alto (>1.20x) en analysis/image o analysis/writer"

**Causa:** Algo está mal configurado.

**Verificar:**
```bash
# En la rama correspondiente:
grep -c "parallel_for" common/include/rendering_engine.hpp
# Debe ser 0 en analysis/image y analysis/writer
# Debe ser >0 en analysis/rendering
```

**Solución:** Revisa que estés en la rama correcta. Cada rama debe tener SOLO un componente paralelo.

---

### Problema: "Imagen diferente a s5-par.ppm del profesor"

**Causa:** Diferentes semillas RNG (normal).

**Explicación:**
- `analysis/rendering` usa rendering paralelo → RNG diferente
- `analysis/image` usa rendering secuencial → MD5 diferente que el profesor
- `analysis/writer` usa rendering secuencial → MD5 diferente que el profesor

**Solución:** Solo compara MD5 entre TUS propias ejecuciones con `--threads 1`.

---

### Problema: "make comando no reconocido"

**Solución:**
```bash
# 1. Verifica que estés en la raíz del proyecto
pwd  # Debe ser /workspace

# 2. Verifica que el makefile existe
ls -la makefile

# 3. Verifica sintaxis del comando
make help  # Si existe
```

---

### Problema: "Scripts no tienen permisos de ejecución"

**Solución:**
```bash
chmod +x scripts/remote/*.sh
make remote-build  # Volver a subir con permisos correctos
```

---

## 📚 Recursos Adicionales

### Documentación Detallada por Rama

- **`GUIA_RAMA_RENDERING.md`**: Detalles de rendering paralelo, flujos, checklist
- **`GUIA_RAMA_IMAGE.md`**: Detalles de image paralelo, Ley de Amdahl, RawImage buffer
- **`GUIA_RAMA_WRITER.md`**: Detalles de writer paralelo, tests de determinismo, I/O bound

### Scripts de Análisis

- **`scripts/analysis/compare.py`**: Compara imágenes PPM (pixel a pixel)
- **`scripts/analysis/analyze_best.py`**: Encuentra mejor configuración en CSV
- **`scripts/analysis/plot_speedup.py`**: Genera gráficas de speedup

### Archivos de Configuración

- **`env.sh`**: Credenciales de Avignon
- **`.password`**: Contraseña (NO COMMITEAR)
- **`CMakeLists.txt`**: Configuración de compilación
- **`res/scene_scripts/scene5.txt`**: Escena de prueba principal
- **`res/config_scripts/config5.txt`**: Configuración de rendering

---

## ⚡ Cheat Sheet Final

### Cambiar de Rama y Probar

```bash
git checkout analysis/rendering
make remote-build
make test-custom
make tail-custom
```

### Flujo Completo (Metodología del Profesor)

```bash
# Paso 1: Hilos
make sweep-threads-first && make fetch-results

# Paso 2: Grain (con hilos óptimos)
make sweep-grain THREADS=56 && make fetch-results

# Paso 3: Escalabilidad (con config óptima)
make sweep-scale PART=static GRAIN=64 && make fetch-results
```

### Flujo Rápido (Tradicional)

```bash
make sweep-opt && make fetch-results
make sweep-scale PART=static GRAIN=64 && make fetch-results
```

---

**¿Necesitas más ayuda?** Revisa la documentación detallada de cada rama o abre un issue en el repositorio.
