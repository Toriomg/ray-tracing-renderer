# Manual de Experimentación: Rama analysis/image

Este documento detalla el procedimiento de pruebas para la rama **"Solo Procesado de Imagen Paralelo"**.

El objetivo científico de esta rama es **aislar el comportamiento de la etapa de post-procesado** (Tone Mapping / Gamma Correction) cuando se somete a paralelismo con TBB, manteniendo el renderizado (trazado de rayos) aislado mediante un buffer intermedio.

---

## 1. Arquitectura del Código en esta Rama

Para este experimento, hemos realizado una refactorización arquitectónica importante (`RawBuffer`):

| Componente | Estado | ¿Afectan los parámetros? | Nota Técnica |
|------------|--------|--------------------------|--------------|
| **Motor** (`rendering_engine`) | **SECUENCIAL** | **NO.** Escribe en `RawBuffer` (doubles). | Simula la carga de generación de datos. |
| **Imagen** (`image_par`) | **PARALELO** (TBB) | **SÍ.** Obedece a `--image-part` y `--image-grain`. | Lee del buffer y procesa en paralelo. |
| **Global** (`main`) | **Híbrido** | **SÍ.** Obedece a `--threads`. | Gestiona las dos etapas. |

### Argumentos de Ejecución Relevantes

En esta rama, los argumentos de renderizado son ignorados o irrelevantes para la optimización, mientras que los de imagen son los protagonistas:

```bash
./render-par scene.txt config.txt out.ppm \
    --render-part <tipo>       # <--- IRRELEVANTE (El render es secuencial/fijo)
    --image-part <tipo>        # <--- CRÍTICO: Controla el particionador de TBB en Stage 2
    --image-grain <n>          # <--- CRÍTICO: Controla el tamaño de grano en Stage 2
    --threads <n>              # <--- Controla los recursos globales
```

---

## 2. Flujo de Trabajo para el Estudio

Al ser la etapa de imagen mucho más rápida que el renderizado, las diferencias de tiempo serán sutiles (milisegundos). El objetivo es demostrar eficiencia en acceso a memoria.

### Paso 0: Preparación

Sube y compila el código refactorizado (Main con Buffer + Imagen Paralela).

```bash
make remote-build
```

---

### Paso 1: Encontrar la Configuración de Memoria Óptima

El procesado de imagen es **Memory Bound** (limitado por velocidad de RAM, no de CPU). Buscamos qué particionador gestiona mejor la caché.

**Lanzar test de optimización:**

```bash
# Nota: Modifica sweep_optimization.sh para que varíe --image-part si no lo hace por defecto
make sweep-opt
```

**Analizar resultados:**

```bash
make fetch-results
python3 scripts/analysis/analyze_best.py logs/results_optimization.csv
```

**Hipótesis esperada:** Es probable que `affinity_partitioner` o `static_partitioner` ganen aquí debido a la naturaleza lineal del acceso a memoria del buffer.

---

### Paso 2: Curva de Escalabilidad (Ley de Amdahl)

Aquí veremos una curva de speedup menos agresiva que en rendering, porque la parte paralela (imagen) representa un porcentaje pequeño del tiempo total.

**Lanzar test de hilos:**

```bash
make sweep-scale PART=static GRAIN=4000  # Usa los valores ganadores del Paso 1
```

---

## 3. Verificación de la Arquitectura RawBuffer

Dado que hemos introducido una estructura intermedia, es vital asegurar que la imagen final es correcta y no una "pantalla negra" o ruido.

**Prueba manual rápida:**

```bash
./scripts/remote/test_custom.sh --image-part static --image-grain 1000
```

Descarga la imagen `out.ppm` resultante y verifica visualmente que se ve igual que la versión secuencial.

### Nota sobre la Validación Visual
Si la imagen resultante parece incorrecta o muy pequeña:
1. **Tamaño del archivo:** Para la **Escena 5** (1800x1200), el archivo `.ppm` debe pesar entre **20MB y 30MB**. Si pesa ~4MB, estás renderizando la Escena 1 (incorrecto para el estudio final).
2. **Imagen Negra:** Si la imagen tiene el tamaño correcto pero es negra, asegúrate de que el `RawBuffer` está pasando los datos y que `fill_from_double` aplica el Gamma correctamente.

---

## 4. Interpretación de Resultados para la Memoria

Esta rama es fundamental para discutir tipos de carga de trabajo:

| Componente | Tipo de Carga | Comportamiento de Escalabilidad |
|------------|---------------|--------------------------------|
| **Rendering** (Rama anterior) | **Compute Bound** | Escala casi linealmente con CPUs. |
| **Imagen** (Esta rama) | **Memory Bound** | Escala peor porque saturamos el ancho de banda de memoria antes que los núcleos de CPU. |

### Cita sugerida para el informe:

> _"Al desacoplar el procesado de imagen mediante un RawBuffer, observamos que la escalabilidad de la etapa de Tone Mapping está limitada por el ancho de banda de memoria, beneficiándose más de particionadores estáticos que reducen el overhead de gestión de tareas, a diferencia del Rendering que prefiere dinámicos."_

---

## 5. Scripts Disponibles y su Función en esta Rama

| Script | Utilidad en esta Rama |
|--------|----------------------|
| `test_custom.sh` | **Alta.** Verificación rápida de que `--image-part` funciona y no hay segfaults. |
| `sweep_optimization.sh` | **Alta.** Sirve para tunear la configuración de memoria (image processing). |
| `sweep_scalability.sh` | **Crítica.** Demuestra la limitación por ancho de banda de memoria. |
| `sweep_matrix.sh` | **Baja.** El motor es fijo (secuencial), solo la imagen varía. |

---

## 6. Checklist de Verificación Final

Antes de dar los datos por buenos, verifica:

- [ ] **Argumentos:** Confirmar que al cambiar `--image-part` en la consola, el programa imprime `Stage 2: Processing image with TBB (parallel)...` con el valor correcto.
- [ ] **Buffer:** Confirmar que no hay segmentation fault al pasar los vectores grandes entre etapas.
- [ ] **Validación Visual:** La imagen de salida tiene colores correctos (Gamma aplicado).
- [ ] **Tiempo Stage 2:** Verificar que en `results_scalability.csv`, el tiempo de Stage 2 varía con los parámetros de imagen.

---

## 7. Estructura Completa del Estudio Experimental

### 7.1. El Control de Variables (Lo que hemos conseguido)

Hemos implementado un sistema de **desacoplamiento arquitectónico** que permite aislar el análisis:

| Parámetro | Propósito en esta Rama (`analysis/image`) | Script que lo Mide |
|-----------|-------------------------------------------|-------------------|
| `--render-part` / `--render-grain` | **CONTROL**. No afecta (motor secuencial). | N/A |
| `--image-part` / `--image-grain` | **CRÍTICO**. Mide el rendimiento del **Post-Procesado Paralelo**. | `sweep_optimization.sh` |
| `--threads <n>` | **ESCALABILIDAD**. Mide la saturación de ancho de banda de memoria. | `sweep_scalability.sh` |

### 7.2. Cobertura de Requisitos

| Requisito del Equipo | Cómo lo cubrimos |
|----------------------|------------------|
| "Aislar el impacto del procesado de imagen" | Cubierto con la arquitectura **RawBuffer**. El rendering escribe datos crudos que luego se procesan en paralelo de forma aislada. |
| "Demostrar limitación por memoria" | Cubierto con `sweep_scalability.sh`. La curva de speedup se aplana antes que en rendering, demostrando que el cuello de botella es el ancho de banda de RAM. |
| "Comparar con rendering paralelo" | Esta rama genera datos complementarios a `analysis/rendering`, permitiendo contrastar cargas **Compute Bound** vs **Memory Bound**. |

### 7.3. Flujo de Experimentación Final (El Orden para el Éxito)

Para vuestra memoria, el orden de los tests es **vital**.

#### **Fase A: Verificación de Integridad (Branch Sanity Check)**

- **Comando:** `make run-custom` (con argumentos `--image-part static --image-grain 1000`)
- **Propósito:** Verificar que la arquitectura `RawBuffer` funciona correctamente y que la imagen generada es visualmente correcta. Este es el **punto de validación** antes de lanzar tests masivos.

#### **Fase B: Optimización de Configuración (Memory Access Pattern)**

- **Comando:** `make sweep-opt`
- **Propósito:** Encontrar qué combinación de particionador y grano minimiza el tiempo de procesado de imagen. **Hipótesis:** `static` o `affinity` deberían ganar por mejor localidad de caché.

#### **Fase C: Estudio de Escalabilidad (Bandwidth Saturation)**

- **Comando:** `make sweep-scale PART=<ÓPTIMO> GRAIN=<ÓPTIMO>`
- **Propósito:** Generar la **curva de Speedup** que demuestra la limitación por ancho de banda de memoria. Comparar con la curva de `analysis/rendering` para contrastar comportamientos.

---

## 8. Datos Esperados y su Interpretación

### 8.1. Speedup Esperado

A diferencia de `analysis/rendering` donde el speedup puede llegar a 30-40x con 56 hilos, aquí esperamos:

- **Speedup máximo:** 3-8x (dependiendo del tamaño de la imagen)
- **Saturación temprana:** Alrededor de 8-16 hilos
- **Razón:** El ancho de banda de memoria se satura antes que los núcleos de CPU

### 8.2. Gráfica Característica

La gráfica de Speedup debería mostrar:
- Inicio: Crecimiento lineal (Stage 2 aprovecha múltiples hilos)
- Media: Desaceleración progresiva (memoria empieza a saturarse)
- Final: Meseta horizontal (añadir más hilos no mejora, todos esperan por RAM)

---

_Última actualización: 2025-12-04 - Rama analysis/image_
