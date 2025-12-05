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
| **Writer** (`ppm_writer`) | **PARALELO** | Aquí es donde TBB actúa formateando el texto. |

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

## 4. Cómo medir el éxito del Writer (Opcional Pro)

Dado que el tiempo total diluye la mejora del writer, podéis fijaros en el log de salida si añadimos cronómetros, o simplemente **argumentar teóricamente**:

> _"La escritura PPM se paralelizó utilizando `static_partitioner` para la conversión de datos a cadena de texto, reduciendo el cuello de botella de formateo antes del volcado a disco."_

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

## 6. Comandos Rápidos

| Comando | Acción |
|---------|--------|
| `make run-custom` | Prueba manual rápida. |
| `make sweep-opt` | Demostrar que el render es indiferente a parámetros. |
| `make sweep-scale` | Curva de speedup (línea casi plana). |
| `make fetch-results` | Descargar CSVs. |

---

_Última actualización: 2025-12-05 - Rama analysis/writer_
