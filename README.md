[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/y54ywNyp)
# Template for render-2025

This repository contains a template for the project assignment in the Computer
Architecture course at Universidad Carlos III de Madrid.


# How to use the scripts
Aquí tienes un `README.md` listo para copiar y pegar en la raíz de tu proyecto. Está diseñado para que cualquier miembro del equipo entienda cómo trabajar con el nuevo flujo automatizado.

---

# 🚀 Manual de Automatización y Despliegue (Makefile)

Este proyecto utiliza un sistema automatizado basado en `Make` para gestionar la sincronización, compilación y ejecución de tareas entre el entorno de desarrollo local (Docker/VSCode) y el clúster remoto (Avignon).

## 🛠️ 1. Configuración Inicial (Solo la primera vez)

Antes de usar los comandos, necesitas definir tus credenciales y rutas.

1.  Crea un archivo llamado **`env.sh`** en la raíz del proyecto.
2.  Copia el siguiente contenido y edita los valores con tu usuario:

```bash
# env.sh
export PROJECT_NAME="render-par"

# Tu usuario de la universidad (ej: a052XXXX)
export REMOTE_USER="a052XXXX"

# Dirección del servidor
export REMOTE_HOST="avignon.lab.inf.uc3m.es"

# Directorio en el servidor donde se guardará el proyecto
# (Asegúrate de que esta carpeta exista o deja que el script la cree)
export REMOTE_DIR="~/rtx"
```

> **Nota:** El archivo `env.sh` suele añadirse al `.gitignore` para no subir credenciales al repositorio.

### 🔑 Configuración de SSH (Recomendado)
Para no tener que escribir tu contraseña cada vez que ejecutes un comando `make`, configura una clave SSH:

```bash
# En tu terminal local (dentro del contenedor o linux)
ssh-keygen -t rsa -b 4096
ssh-copy-id $REMOTE_USER@$REMOTE_HOST
```

---

## ⚡ 2. Comandos Principales (Workflow)

Ejecuta estos comandos desde la terminal en la raíz del proyecto.

### 📡 Sincronización y Compilación Remota

| Comando | Descripción |
| :--- | :--- |
| **`make deploy`** | Sube los archivos modificados al servidor usando `rsync`. Es incremental (solo sube lo nuevo) y muy rápido. |
| **`make remote-build`** | Sube los cambios (`deploy`) y lanza un trabajo a la cola de **Slurm** para compilar el proyecto en el servidor. |

### 🧪 Ejecución de Pruebas

| Comando | Descripción |
| :--- | :--- |
| **`make gen-tests`** | Ejecuta el script de Python local para generar/actualizar los scripts `.sh` de pruebas en `tests_de_config/` y `tests_de_escenas/`. |
| **`make remote-run-all`** | Sube los scripts generados y **lanza todos los trabajos** a la cola del clúster (`sbatch`) de una sola vez. |
| **`make all-remote`** | El "Botón Mágico": Hace `deploy` -> `remote-build` -> `remote-run-all`. Úsalo cuando hayas hecho cambios en el código C++ y quieras testear todo. |

### 📥 Recopilación de Resultados

| Comando | Descripción |
| :--- | :--- |
| **`make fetch`** | Descarga del servidor el CSV consolidado de resultados y las imágenes `.ppm` generadas a tu carpeta local `out/`. |
| **`make clean-remote`** | (Opcional) Limpia la carpeta de construcción en el servidor por si hay errores de caché corrupta. |

---

## 🔄 Flujo de Trabajo Típico

Imagina que has modificado el código C++ para mejorar el rendimiento. Los pasos serían:

1.  **Subir y Compilar:**
    ```bash
    make remote-build
    ```
    *(Verás en la terminal si se envió el trabajo. Puedes usar `squeue -u tu_usuario` en una terminal SSH para ver si terminó).*

2.  **Lanzar Batería de Pruebas:**
    ```bash
    make remote-run-all
    ```

3.  **Esperar resultados...** (Tómate un café mientras Slurm trabaja).

4.  **Descargar Datos:**
    ```bash
    make fetch
    ```

5.  **Analizar en Local:**
    Ahora tienes los archivos en `out/`. Puedes abrir el CSV en Excel o usar los scripts de Python locales para comparar imágenes.

---

## 📂 Estructura de Scripts

Para mantener el orden, no modifiques el `Makefile` directamente si no es necesario. Edita los scripts correspondientes:

*   **`scripts/deploy/sync.sh`**: Controla qué archivos se suben (y cuáles se ignoran como `.git` o `build`).
*   **`scripts/remote/build.sh`**: Comandos que ejecuta Slurm para compilar (CMake settings).
*   **`scripts/generation/genTests.py`**: Script que crea los tests. Si quieres cambiar parámetros (más rayos, resolución, etc.), edita esto y corre `make gen-tests`.
*   **`scripts/analysis/`**: Scripts para comparar imágenes o generar gráficas en local.