# Guía Rápida: Cómo Correr Tests en Avignon

## 📋 Comandos Principales (Orden de Uso)

### 1️⃣ **Compilar en Avignon** (siempre primero)
```bash
make remote-build
```
Sube el código y compila en el cluster. **Hazlo cada vez que cambies de rama o modifiques código.**

---

### 2️⃣ **Test Personalizado** (EL MÁS IMPORTANTE - JUEGA CON ESTE)
```bash
make test-custom
```

Este comando ejecuta `scripts/remote/test_custom.sh` donde **PUEDES MODIFICAR TODO**:

**Edita este archivo antes de ejecutar:**
```bash
nano scripts/remote/test_custom.sh
```

**Parámetros que puedes tocar:**
```bash
# Dentro del archivo test_custom.sh verás:

PARTITIONER="static"      # Opciones: auto, simple, static, affinity
GRAIN_SIZE="64"           # Valores típicos: 0, 1, 32, 64, 128, 256, 512, 1024
THREADS="56"              # Valores típicos: 1, 2, 4, 8, 16, 28, 56, 112
```

**¿Qué cambia según la rama?**

| Rama | Parámetros que FUNCIONAN | Parámetros que NO HACEN NADA |
|------|--------------------------|-------------------------------|
| **analysis/rendering** | `--render-part`, `--render-grain`, `--threads` | `--image-part`, `--image-grain` |
| **analysis/image** | `--image-part`, `--image-grain`, `--threads` | `--render-part`, `--render-grain` |
| **analysis/writer** | `--threads` (solo para writer) | `--render-part`, `--image-part`, `--render-grain`, `--image-grain` |

---

### 3️⃣ **Ver Resultados del Test**
```bash
make tail-custom
```
Muestra el log del último test. Busca:
- Tiempo de ejecución
- Energía consumida
- Mensajes de error

---

### 4️⃣ **Barrido de Optimización** (encontrar mejor config)
```bash
make sweep-opt
```
Prueba **todas las combinaciones** de partitioner y grain para encontrar la más rápida.

**Resultado:** CSV con tiempos → Analiza con:
```bash
make fetch-results
python3 scripts/analysis/analyze_best.py logs/results_optimization.csv
```

---

### 5️⃣ **Barrido de Escalabilidad** (curva speedup)
```bash
# Primero encuentra la mejor config con sweep-opt, luego:
make sweep-scale PART=static GRAIN=64
```
Prueba con diferentes números de hilos (1, 2, 4, 8, ..., 112) usando la config óptima.

**Resultado:** CSV con tiempos vs threads → Haz gráfica de speedup.

---

## 🔧 Ejemplo Práctico: Probar Diferentes Configuraciones

**Escenario:** Quiero probar `affinity` con grain `128` y 28 hilos.

```bash
# 1. Editar configuración
nano scripts/remote/test_custom.sh
# Cambiar a:
#   PARTITIONER="affinity"
#   GRAIN_SIZE="128"
#   THREADS="28"

# 2. Ejecutar
make test-custom

# 3. Ver resultado
make tail-custom
```

---

## 📊 Diferencias entre Ramas

### **analysis/rendering** (Motor Paralelo)
- **QUÉ ES PARALELO:** Trazado de rayos (rendering_engine)
- **PARAMETROS IMPORTANTES:** `--render-part`, `--render-grain`
- **OBJETIVO:** Medir speedup del rendering

```bash
# En test_custom.sh de esta rama:
RENDER_PARTITIONER="static"
RENDER_GRAIN="64"
THREADS="56"
```

---

### **analysis/image** (Procesado Paralelo)
- **QUÉ ES PARALELO:** Conversión a uint8 + gamma (image_par)
- **PARAMETROS IMPORTANTES:** `--image-part`, `--image-grain`
- **OBJETIVO:** Medir speedup del post-procesado
- **NOTA:** El speedup será **mínimo** porque la imagen es muy rápida (Ley de Amdahl)

```bash
# En test_custom.sh de esta rama:
IMAGE_PARTITIONER="static"
IMAGE_GRAIN="1024"    # Granos grandes porque la tarea es ligera
THREADS="56"
```

---

### **analysis/writer** (Writer Paralelo)
- **QUÉ ES PARALELO:** Escritura PPM (ppm_writer)
- **PARAMETROS IMPORTANTES:** `--threads` (no hay partitioner/grain para writer)
- **OBJETIVO:** Medir speedup de la escritura
- **NOTA:** Speedup limitado (Memory Bound)

```bash
# En test_custom.sh de esta rama:
THREADS="56"
```

---

## 🚨 Errores Comunes

### "No such file or directory"
**Solución:** Compilaste? → `make remote-build`

### "Job submission failed"
**Solución:** Revisa que estás en Avignon con `ssh avignon`

### "Resultados no cambian"
**Solución:** ¿Modificaste `test_custom.sh` en la rama correcta? ¿Recompilaste después?

---

## 📁 Archivos Importantes

```
scripts/remote/
├── test_custom.sh          ← EDITA ESTE para tests rápidos
├── sweep_optimization.sh   ← Barrido de configs (auto)
├── sweep_scalability.sh    ← Barrido de threads (auto)
└── sweep_matrix.sh         ← Barrido completo (avanzado)

logs/
├── results_optimization.csv   ← Resultados de sweep-opt
├── results_scalability.csv    ← Resultados de sweep-scale
└── custom_*.out               ← Logs de test-custom
```

---

## ⚡ Cheat Sheet Rápido

```bash
# Flujo típico de trabajo:
git checkout analysis/rendering     # Cambiar de rama
make remote-build                   # Compilar
nano scripts/remote/test_custom.sh  # Ajustar parámetros
make test-custom                    # Ejecutar test
make tail-custom                    # Ver resultado

# Para generar datos del paper:
make sweep-opt                      # Encontrar mejor config
make fetch-results                  # Descargar CSVs
make sweep-scale PART=static GRAIN=64  # Generar curva speedup
```

---

## 💡 Consejo Final

**Para entender QUÉ hace cada rama:**
- Lee `GUIA_RAMA_RENDERING.md` (5 min)
- Lee `GUIA_RAMA_IMAGE.md` (5 min)  
- Lee `GUIA_RAMA_WRITER.md` (si existe)

**O simplemente mira la tabla de arriba** 👆 y sabrás qué parámetros tocar en cada rama.

---

**¿Dudas?** Revisa los `.md` de cada rama.
