[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/y54ywNyp)
# Template for render-2025

This repository contains a template for the project assignment in the Computer
Architecture course at Universidad Carlos III de Madrid.


# How to use the scripts


# 🚀 Manual de Automatización y Despliegue (Makefile)

Este proyecto utiliza un sistema automatizado basado en `Make` para gestionar la sincronización, compilación y ejecución de tareas entre el entorno de desarrollo local (Docker/VSCode) y el clúster remoto (Avignon).

## 🛠️ 1. Configuración Inicial (Solo la primera vez)

Antes de usar los comandos, necesitas definir tus credenciales y rutas.

1.  Edita el archivo llamado **`env.sh`** en la raíz del proyecto.
2.  Copia el siguiente contenido y edita los valores con tu usuario:

```bash
# env.sh
# Tu usuario de la universidad (ej: a052XXXX)
REMOTE_USER=a0522253
REMOTE_HOST=avignon.lab.inf.uc3m.es
# Directorio en el servidor donde se guardará el proyecto
# (deja que el script la cree)
REMOTE_DIR=rtx
```

> **Nota:** El archivo `env.sh` suele añadirse al `.gitignore` para no subir credenciales al repositorio.

### Archivo de Contraseña (`.password`)
Para evitar escribir la contraseña constantemente, el sistema usa `sshpass`, instálalo.
1. Crea un archivo llamado `.password` en la raíz.
2. Escribe tu contraseña de la Avignon dentro (solo la contraseña, sin espacios extra).
3. **Importante:** Asegúrate de que `.password` está en tu archivo `.gitignore`.

```bash
# Mejor hazlo en el IDE porque con la terminal se queda en su historial
echo "tu_contraseña_aqui" > .password
# Ya está incluido pero compruebalo por si acaso.
echo ".password" >> .gitignore
```

---

## ⚡ 2. Comandos Principales (Workflow)

#### 🤖 Ejecución Automática (Recomendado)

| Comando | Descripción |
| :--- | :--- |
| **`make auto-jd`** | **Flujo completo "Zero-Touch":** <br>1. Sube y **Compila** el código (espera a terminar).<br>2. Limpia logs y **Ejecuta** los tests (espera a terminar).<br>3. **Descarga** resultados.<br>4. **Valida** y genera el CSV. |

#### 📡 Conexión y Compilación (Manual)

| Comando | Descripción |
| :--- | :--- |
| **`make deploy`** | Empaqueta y sube el código al servidor. |
| **`make remote-build`** | Sube cambios y manda compilar en el clúster (Slurm). |

#### 🧪 Ejecución Manual

| Comando | Descripción |
| :--- | :--- |
| **`make run-jd`** | Lanza el test a la cola de Slurm y devuelve el control inmediatamente (no espera). |
| **`make tail-jd`** | Muestra en tiempo real el log de salida del test (`tail -f`). |
| **`make run-jd-wait`** | Lanza el test y bloquea la terminal hasta que Slurm termine la ejecución. |

#### 📥 Descarga y Análisis

| Comando | Descripción |
| :--- | :--- |
| **`make fetch-all`** | Descarga todo (`ppm` + `txt`) y **ejecuta el pipeline de validación**. |
| **`make pipeline`** | Ejecuta la comparación de imágenes y extracción de tiempos localmente. |

#### Resultados
Si la validación de imágenes es correcta ("VÁLIDO"), el script generará automáticamente un CSV con los tiempos:
*   **Archivo:** `memoria/graphs/[NOMBRE-RAMA-GIT].csv`
*   **Logs:** `logs/txt/run-test-jd.out`
*   **Imágenes:** `logs/images/`

## 📂 Estructura de Scripts

Para mantener el orden, no modifiques el `Makefile` directamente si no es necesario. Edita los scripts correspondientes:

*   **`scripts/deploy/sync.sh`**: Controla qué archivos se suben (y cuáles se ignoran como `.git`, `.password` `env.sh` o `build`).
*   **`scripts/remote/build.sh`**: Comandos que ejecuta Slurm para compilar (CMake settings).
*   **`scripts/remote/run-test-jd.sh`**: Script que ejecuta los casos de prueba.
*   **`scripts/generation/genTests.py`**: Script que crea los tests. Si quieres cambiar parámetros (más rayos, resolución, etc.), edita esto y corre `make gen-tests`.
*   **`scripts/analysis/`**: Scripts para comparar imágenes o generar gráficas en local.
