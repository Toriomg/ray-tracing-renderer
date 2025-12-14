# Manual de Experimentación: Rama analysis/image

Este documento detalla la metodología de pruebas para la rama **"Solo Procesado de Imagen Paralelo"**.

A diferencia de la rama de renderizado, aquí nos enfrentamos a un reto: la parte que queremos optimizar (el procesado de imagen) es muy rápida en comparación con el trazado de rayos. Por ello, hemos cambiado la arquitectura del programa para poder medirla de forma aislada.

---

## 1. Arquitectura del Código (Buffer Intermedio)

Para aislar el rendimiento de la imagen, el programa ahora funciona en **dos etapas**:

- **Etapa 1 (Carga Base):** El motor (`rendering_engine`) calcula los rayos de forma **SECUENCIAL** y guarda los datos "crudos" (doubles) en un buffer temporal (`RawImage`). Esto simula una carga de trabajo pesada y constante.
- **Etapa 2 (Optimización):** La clase `ImagePar` lee ese buffer, aplica la corrección Gamma y convierte a colores (0-255) utilizando **PARALELISMO** (TBB).

| Componente | Estado | ¿Afectan los parámetros? |
|------------|--------|--------------------------|
| **Motor** | **SECUENCIAL** | **NO.** Ignora `--render-part`. Siempre tarda lo mismo (~25s). |
| **Imagen** | **PARALELO** | **SÍ.** Obedece a `--image-part` y `--image-grain`. Aquí buscamos la mejora. |

---

## 2. Guía de Scripts y Experimentación

### Argumentos Clave

En esta rama, debéis centraros **exclusivamente** en los parámetros de imagen:

```bash
./render-par ... --image-part static --image-grain 1024 --threads 56
```

**Valores clave de hilos:**
- **28 hilos:** 1 socket completo (14 cores × 2 threads)
- **56 hilos:** 2 sockets físicos (máximo paralelismo real)
- **112 hilos:** Máximo posible (con hyperthreading)
- **120 hilos:** 7% sobre el máximo (verificar sobrecarga)

---

### 🎯 **FLUJO A: Metodología Recomendada (Hilos Primero → Granularidad Después)**

#### **Paso 0: Validación (Sanity Check)**

Antes de lanzar pruebas largas, verificad que el sistema de doble etapa funciona.

```bash
make remote-build
make test-custom
make tail-custom
```

**Verificación:** Debéis ver en el log:
- `Stage 1: Rendering to RawImage buffer (sequential)...`
- `Stage 2: Processing image with TBB (parallel, partitioner: ...)...`

---

#### **Paso 1: Exploración Inicial de Hilos**

Probar con 28, 56, 112 y 120 hilos usando `auto_partitioner`:

```bash
make sweep-threads-first
```

**¿Qué hace?** Prueba los 4 valores clave de hilos con TBB decidiendo el grain automáticamente.

**⚠️ ADVERTENCIA:** Las diferencias de tiempo serán **mínimas** (~0.1-0.2s) porque el procesado de imagen es <1% del tiempo total.

**Descargar y analizar:**

```bash
make fetch-results
# Busca el mejor tiempo en logs/results_threads_first.csv
# Ejemplo: Si 56 hilos es óptimo, anótalo
```

---

#### **Paso 2: Explorar Granularidad (con hilos fijos)**

Con el número óptimo de hilos encontrado, probar diferentes grains:

```bash
make sweep-grain THREADS=56
```

**¿Qué hace?** 
- Fija los hilos en 56
- Prueba grains: 56, 28, 14, 7, 3, 1, 0 (reducción por mitad)
- **PLUS:** Prueba granos grandes (1024, 2048, 4096, 8192) típicos de procesado de imagen
- Prueba con los 4 partitioners

**Descargar y analizar:**

```bash
make fetch-results
# Busca la mejor combinación en logs/results_grain_sweep.csv
# Ejemplo: static con grain=2048
```

---

#### **Paso 3: Curva de Escalabilidad Final**

Con la configuración óptima:

```bash
make sweep-scale PART=static GRAIN=2048
```

**¿Qué hace?** Genera la curva de speedup probando hilos: 1, 2, 4, 8, 16, 28, 56, 60, 64, ..., 120

**⚠️ RESULTADO ESPERADO:** Gráfica casi plana (Ley de Amdahl). El tiempo total NO mejorará significativamente porque la imagen es <1% del tiempo.

---

### ⚡ **FLUJO B: Tradicional (Más Rápido)**

#### **Paso 1: Optimización de Imagen**

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

#### **Paso 2: Escalabilidad**

```bash
make sweep-scale PART=static GRAIN=4096
```

**Descargar:**

```bash
make fetch-results
```

---

### 🔬 **FLUJO C: Verificación Exhaustiva (Opcional)**

```bash
sbatch scripts/remote/sweep_matrix.sh
```

**Configuración recomendada en el script:**
- `RENDER_PARTS=("auto")` ← Da igual, es secuencial
- `IMAGE_PARTS=("static" "affinity" "auto")` ← Lo que queremos probar
- `IMAGE_GRAINS=(0 1024 2048 4096 8192)` ← Granos grandes para memory-bound



---

## 3. Scripts Disponibles y su Función en esta Rama

| Script | Utilidad en esta Rama | Argumentos que usa | Comando Make |
|--------|----------------------|--------------------|--------------|
| `test_custom.sh` | **Alta.** Prueba manual rápida, verifica Stage 1 y Stage 2. | `--image-part`, `--image-grain`, `--threads` | `make test-custom` |
| `sweep_threads_first.sh` | **🆕 Crítica (Flujo A).** Explora 28, 56, 112, 120 hilos con auto_partitioner. | `--image-part auto`, `--image-grain 0`, `--threads` | `make sweep-threads-first` |
| `sweep_grain.sh` | **🆕 Crítica (Flujo A).** Con hilos fijos, prueba grains (threads→threads/2→...→1→0) + grandes (1024-8192). | `--image-part`, `--image-grain`, `--threads <fijo>` | `make sweep-grain THREADS=56` |
| `sweep_optimization.sh` | **Crítica (Flujo B).** Encuentra mejor partitioner+grain con 56 hilos fijos. | `--image-part`, `--image-grain`, `--threads 56` | `make sweep-opt` |
| `sweep_scalability.sh` | **Crítica.** Genera curva de speedup (1→120 hilos, paso=4). **Mejora mínima esperada.** Ahora acepta rango personalizado. | `--image-part`, `--image-grain`, `--threads` | `make sweep-scale PART=X GRAIN=Y [START=N END=M STEP=S]` |
| `sweep_matrix.sh` | **Media.** Combinaciones exhaustivas (útil para verificación). | Varios | Manual con `sbatch` |

**Notas importantes para analysis/image:**
- ✅ `sweep_scalability.sh` ahora prueba hasta **120 hilos** (antes solo hasta 112)
- ✅ `sweep_scalability.sh` hace barrido fino de 56→120 con paso=4 (56, 60, 64, 68, ..., 120)
- ✅ **NUEVO:** `sweep_scalability.sh` acepta rangos personalizados: `START=56 END=112 STEP=2`
- ✅ **MEJORAS DE ROBUSTEZ:** Todos los scripts usan `perf -r 5` (promedio de 5 ejecuciones), error handling y sync
- ✅ `sweep_optimization.sh` ahora incluye **granos grandes** (1024, 2048, 4096, 8192) típicos de memory-bound
- ✅ `sweep_grain.sh` implementa reducción por mitad PLUS granos grandes como pide el profesor
- ⚠️ **DIFERENCIA CLAVE:** Esta rama usa `--image-part` y `--image-grain` (NO `--render-part`)

---

## 4. Interpretación de Resultados (Para la Memoria)

Cuando analicéis los datos de `results_scalability.csv`, observad esto:

### Tiempo Total casi plano

Si con 1 hilo tardamos **25.5s** y con 112 hilos tardamos **25.1s**, **ES UN RESULTADO CORRECTO**.

**Explicación:**

> _"La paralelización del módulo ImagePar fue exitosa, pero su impacto en el tiempo de ejecución global es despreciable debido a que representa menos del 1% de la carga computacional total (Ley de Amdahl). El cuello de botella principal sigue siendo el trazado de rayos."_

### Eficiencia

Es probable que la escalabilidad de la imagen se sature muy rápido (a los **4-8 hilos**) porque es una tarea **Memory Bound** (limitada por la velocidad de escritura en RAM, no por la potencia de cálculo).

| Componente | Tipo de Carga | Comportamiento de Escalabilidad |
|------------|---------------|--------------------------------|
| **Rendering** (Rama anterior) | **Compute Bound** | Escala casi linealmente con CPUs (30-40x speedup). |
| **Imagen** (Esta rama) | **Memory Bound** | Escala pobremente (3-8x speedup máximo, saturación temprana). |

---

## 5. Flujo de Experimentación Recomendado (Metodología del Profesor)

Para vuestra memoria, sigue **este orden científico**:

### **Fase 1: Exploración Inicial de Hilos** 🎯

```bash
make remote-build
make sweep-threads-first
make fetch-results
```

**Resultado:** `logs/results_threads_first.csv` con tiempos de 28, 56, 112, 120 hilos.

**Decisión:** Identifica el número óptimo (ejemplo: 56 hilos).

**⚠️ IMPORTANTE:** Los tiempos serán casi iguales (~25.0-25.5s) porque la imagen es <1% del tiempo total.

---

### **Fase 2: Optimización de Granularidad** 🔬

```bash
make sweep-grain THREADS=56
make fetch-results
```

**Resultado:** `logs/results_grain_sweep.csv` con todas las combinaciones partitioner × grain.

**Decisión:** Identifica la mejor config (ejemplo: static, grain=2048).

**Nota:** Granos grandes (1024-8192) funcionan mejor en tareas memory-bound ligeras.

---

### **Fase 3: Curva de Escalabilidad (Ley de Amdahl)** 📈

```bash
make sweep-scale PART=static GRAIN=2048
make fetch-results
```

**Resultado:** `logs/results_scalability.csv` con speedup de 1→120 hilos (paso=4).

**Para la Memoria:** Gráfica casi plana demostrando que paralelizar <1% del código no mejora el tiempo total (Ley de Amdahl).

---

### **Fase 4 (Opcional): Verificación Exhaustiva** 🚀

```bash
sbatch scripts/remote/sweep_matrix.sh
```

**Propósito:** Tabla completa para demostrar: _"Probamos todas las configuraciones de imagen, pero el impacto es mínimo debido a Amdahl"_.

---

## 6. Comandos Rápidos

| Comando | Acción |
|---------|--------|
| `make test-custom` | Prueba manual rápida (verifica Stage 1 y Stage 2). |
| `make sweep-threads-first` | Exploración inicial de hilos (28, 56, 112, 120). |
| `make sweep-grain THREADS=56` | Optimización de granularidad con hilos fijos. |
| `make sweep-opt` | Barrido de optimización (flujo tradicional). |
| `make sweep-scale PART=X GRAIN=Y` | Curva de escalabilidad (para demostrar Amdahl). |
| `make fetch-results` | Descargar CSVs. |
| `python3 scripts/analysis/plot_results.py` | Generar gráficas. |

---

## 7. Checklist de Verificación

Antes de dar los datos por buenos, verifica:

- [ ] **Logs correctos:** Al ejecutar `make tail-custom`, ves `Stage 1: Rendering...` seguido de `Stage 2: Processing image with TBB...`
- [ ] **Imagen válida:** El archivo `.ppm` generado tiene el tamaño esperado (para scene5: ~20-30MB) y se ve correctamente al abrirlo
- [ ] **Argumentos efectivos:** Cambiar `--image-part` o `--image-grain` produce diferencias medibles en los logs
- [ ] **Sin crashes:** No hay segmentation faults al pasar los vectores entre `RawImage` y `ImagePar`
- [ ] **Scripts:** `sweep_scalability.sh` prueba hasta 120 hilos (no solo 112) ✅
- [ ] **Scripts:** `sweep_grain.sh` prueba granos grandes (1024-8192) además de reducción por mitad ✅
- [ ] **Scripts:** Barrido fino 56→120 con paso=4 en `sweep_scalability.sh` ✅
- [ ] **Parámetros correctos:** Todos los scripts usan `--image-part` y `--image-grain` (NO `--render-part`) ✅

---

## 8. Datos Esperados y su Interpretación

### Speedup Esperado (Comparación con analysis/rendering)

| Rama | Componente Paralelo | Speedup Máximo Esperado | Saturación | Tipo de Carga |
|------|---------------------|-------------------------|------------|---------------|
| **analysis/rendering** | Motor de rayos | **30-40x** con 56 hilos | ~56 hilos | Compute Bound |
| **analysis/image** (esta) | Procesado de imagen | **3-8x** en Stage 2, **<1.05x** total | ~8-16 hilos | Memory Bound |

**⚠️ CRÍTICO:** En esta rama, el **tiempo total** apenas mejorará porque:
- Stage 1 (secuencial): ~25s (99% del tiempo)
- Stage 2 (paralelo): ~0.1-0.2s (1% del tiempo)
- **Amdahl dice:** Speedup máximo teórico = 1 / (0.99 + 0.01/∞) ≈ **1.01x**

### Gráfica Característica del Tiempo Total

La gráfica de Tiempo vs Threads debería mostrar:
- **1 hilo:** ~25.5s
- **28 hilos:** ~25.3s
- **56 hilos:** ~25.2s
- **112 hilos:** ~25.1s
- **120 hilos:** ~25.1s (igual o ligeramente peor por overhead)

**Conclusión para la Memoria:**
> _"La paralelización del procesado de imagen fue técnicamente exitosa (speedup de 5x en Stage 2 aislado), pero su impacto en el rendimiento global es despreciable (<2% de mejora) debido a que representa menos del 1% del tiempo total de ejecución. Este resultado confirma la Ley de Amdahl: optimizar componentes minoritarios no afecta significativamente al tiempo total."_

### Gráfica del Stage 2 Aislado (Si la medís)

Si conseguís aislar solo el tiempo de Stage 2:
- **Inicio (1-4 hilos):** Crecimiento casi lineal
- **Media (8-16 hilos):** Desaceleración progresiva (memoria se satura)
- **Final (>16 hilos):** Meseta horizontal (memory-bound, todos esperan por RAM)

---

## 9. Cobertura de Requisitos del Profesor ✅

| Requisito del Profesor | Cómo lo cubrimos | Script |
|------------------------|------------------|--------|
| **"Primero establecer número de hilos"** | ✅ `sweep_threads_first.sh` prueba 28, 56, 112, 120 con auto | `make sweep-threads-first` |
| **"Empezar con 4 valores: 28, 56, 112, 120"** | ✅ Implementado exactamente en `sweep_threads_first.sh` | `make sweep-threads-first` |
| **"Después probar granularidad"** | ✅ `sweep_grain.sh` con hilos fijos, grain=threads→mitad→...+grandes | `make sweep-grain THREADS=X` |
| **"Primer valor de grain = número de hilos"** | ✅ `sweep_grain.sh` empieza con `GRAINS=($OPTIMAL_THREADS)` | `make sweep-grain THREADS=56` |
| **"Reducir a la mitad hasta que empeore"** | ✅ Bucle automático: 56→28→14→7→3→1→0 | `make sweep-grain THREADS=56` |
| **"Rango de 56 a 112 con step de 4"** | ✅ `sweep_scalability.sh` usa `seq 56 4 120` | `make sweep-scale PART=X GRAIN=Y` |
| **"Probar hasta 120 hilos"** | ✅ Añadido a `sweep_scalability.sh` y `sweep_threads_first.sh` | Ambos scripts |
| **"Probar partitioners"** | ✅ Todos los scripts prueban auto, simple, static, affinity | Todos los sweep |
| **"Combinar granularidad con partitioners"** | ✅ `sweep_grain.sh` hace producto cartesiano | `make sweep-grain THREADS=X` |
| **"Probar granos grandes (memory-bound)"** | ✅ `sweep_grain.sh` añade 1024, 2048, 4096, 8192 | `make sweep-grain THREADS=X` |

---

## 10. Resumen de Cambios

### ✅ Últimas Actualizaciones (2025-12-06)

**Mejoras de Robustez en Mediciones:**
- ✅ **perf stat -r 5:** Todos los scripts ejecutan 5 repeticiones y promedian (reduce ruido)
- ✅ **Error handling:** Validación de salida de perf y datos extraídos
- ✅ **sync después de cada escritura:** Previene pérdida de datos en crashes
- ✅ **Logging mejorado:** Mensajes de error explícitos en stderr

**Flexibilidad en Escalabilidad:**
- ✅ **sweep_scalability.sh** ahora acepta rangos personalizados:
  ```bash
  make sweep-scale PART=static GRAIN=2048 START=56 END=112 STEP=2
  # Ejecuta: 56, 58, 60, ..., 112
  ```
- ✅ Modo híbrido por defecto (sin START): 1, 2, 4, 8, 16, 28 + seq(56,STEP,END)
- ✅ Modo personalizado (con START): seq(START,STEP,END)

### ✅ Correcciones Previas (2025-12-05)

1. **sweep_scalability.sh:**
   - ✅ Añadido 120 hilos al barrido
   - ✅ Barrido fino: `seq 56 4 120` (56, 60, 64, 68, ..., 120)
   - ✅ Cambiado a `--image-part` y `--image-grain`
   - ✅ Ahora flexible con argumentos opcionales

2. **sweep_optimization.sh:**
   - ✅ Añadidos granos grandes: 1024, 2048, 4096, 8192 (típicos de memory-bound)
   - ✅ Mantiene reducción por mitad desde threads: 56, 28, 14, 7, 1, 0
   - ✅ Fija 56 hilos con `--threads 56` explícitamente
   - ✅ Usa `--image-part` y `--image-grain`
   - ✅ Mejoras de robustez aplicadas

3. **sweep_threads_first.sh (NUEVO):**
   - ✅ Explora 28, 56, 112, 120 hilos con `auto_partitioner`
   - ✅ Implementa "Paso 1" de la metodología del profesor
   - ✅ Advertencia: mejoras mínimas esperadas (imagen <1% del tiempo)

4. **sweep_grain.sh (NUEVO):**
   - ✅ Con hilos fijos, prueba grains: threads→threads/2→...→1→0
   - ✅ **PLUS:** Añade granos grandes (1024, 2048, 4096, 8192)
   - ✅ Implementa "Paso 2" de la metodología del profesor
   - ✅ Producto cartesiano: 4 partitioners × (reducción mitad + grandes)
   - ✅ Mejoras de robustez aplicadas

5. **Makefile:**
   - ✅ Añadidos comandos: `make sweep-threads-first`, `make sweep-grain THREADS=X`
   - ✅ Actualizado `sweep-scale` para aceptar START, END, STEP
   - ✅ Mensajes informativos sobre naturaleza de analysis/image

6. **Documentación:**
   - ✅ Explicados ambos flujos (A: hilos primero, B: tradicional)
   - ✅ Tabla de cobertura de requisitos del profesor
   - ✅ Advertencias sobre resultados esperados (Ley de Amdahl)
   - ✅ Comparación con analysis/rendering (compute vs memory bound)

### 📊 Ahora Cumplimos TODOS los Requisitos

| Requisito | Estado |
|-----------|--------|
| Probar 28, 56, 112, 120 hilos | ✅ |
| Empezar grain = número de hilos | ✅ |
| Reducir grain a la mitad | ✅ |
| Probar granos grandes (memory-bound) | ✅ |
| Barrido fino 56→120 (paso=4) | ✅ |
| Flujo: hilos primero → grain después | ✅ |
| Parámetros correctos (--image-part) | ✅ |

---

_Última actualización: 2025-12-06 - Mejoras de robustez y flexibilidad añadidas_
