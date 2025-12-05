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

### Flujo de Trabajo Recomendado

#### **Paso 1: Validación (Sanity Check)**

Antes de lanzar pruebas largas, verificad que el sistema de doble etapa funciona.

```bash
make remote-build
make run-custom
make tail-custom
```

**Verificación:** Debéis ver en el log:
- `Stage 1: Rendering to RawImage buffer (sequential)...`
- `Stage 2: Processing image with TBB (parallel, partitioner: ...)...`

---

#### **Paso 2: Optimización de Imagen (sweep_matrix.sh)**

Como el procesado de imagen es muy rápido (<0.1s), los tests estándar pueden tener mucho "ruido". Usaremos la matriz para probar muchas veces.

**Configuración:** Editad `scripts/remote/sweep_matrix.sh`:
- `RENDER_PARTS=("auto")` ← Da igual, es secuencial.
- `IMAGE_PARTS=("static" "affinity" "auto")` ← Lo que queremos probar.
- `IMAGE_GRAINS=(0 1024 4096)` ← Probad granos grandes, ya que la tarea es ligera.

**Lanzar:**
```bash
sbatch scripts/remote/sweep_matrix.sh
```

---

#### **Paso 3: Escalabilidad (sweep_scalability.sh)**

**⚠️ ADVERTENCIA:** En esta gráfica, **NO veréis una mejora espectacular** del tiempo total (Time vs Threads), porque el 99% del tiempo se gasta en la Etapa 1 (Secuencial).

**Qué buscamos:** Demostrar la **Ley de Amdahl**. Aunque aceleremos la imagen infinitamente con 112 hilos, el tiempo total nunca bajará de lo que tarda el motor secuencial.

**Ejecución:**
```bash
# Usad la mejor configuración de imagen encontrada
make sweep-scale PART=static GRAIN=4096
```



---

## 3. Interpretación de Resultados (Para la Memoria)

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

## 4. Comandos Rápidos

| Comando | Acción |
|---------|--------|
| `make run-custom` | Prueba manual rápida. |
| `sbatch scripts/remote/sweep_matrix.sh` | Barrido de optimización (enfocado en Imagen). |
| `make sweep-scale` | Test de hilos (para demostrar Amdahl). |
| `make fetch-results` | Descargar CSVs. |
| `python3 scripts/analysis/plot_results.py` | Generar gráficas. |

---

## 5. Checklist de Verificación

Antes de dar los datos por buenos, verifica:

- [ ] **Logs correctos:** Al ejecutar `make tail-custom`, ves `Stage 1: Rendering...` seguido de `Stage 2: Processing image with TBB...`
- [ ] **Imagen válida:** El archivo `.ppm` generado tiene el tamaño esperado (para scene5: ~20-30MB) y se ve correctamente al abrirlo
- [ ] **Argumentos efectivos:** Cambiar `--image-part` o `--image-grain` produce diferencias medibles en los logs
- [ ] **Sin crashes:** No hay segmentation faults al pasar los vectores entre `RawImage` y `ImagePar`

---

## 6. Datos Esperados y su Interpretación

### Speedup Esperado

A diferencia de `analysis/rendering` donde el speedup puede llegar a **30-40x** con 56 hilos, aquí esperamos:

- **Speedup máximo:** 3-8x (dependiendo del tamaño de la imagen)
- **Saturación temprana:** Alrededor de 8-16 hilos
- **Razón:** El ancho de banda de memoria se satura antes que los núcleos de CPU

### Gráfica Característica

La gráfica de Speedup debería mostrar:
- **Inicio:** Crecimiento lineal (Stage 2 aprovecha múltiples hilos)
- **Media:** Desaceleración progresiva (memoria empieza a saturarse)
- **Final:** Meseta horizontal (añadir más hilos no mejora, todos esperan por RAM)

---

_Última actualización: 2025-12-05 - Rama analysis/image_
