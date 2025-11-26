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

Ejecuta estos comandos desde la terminal en la raíz del proyecto.

#### 📡 Conexión y Compilación

| Comando | Descripción |
| :--- | :--- |
| **`make deploy`** | Empaqueta y sube el proyecto al servidor usando `tar` sobre `ssh` (no requiere rsync). |
| **`make remote-build`** | Sube cambios (`deploy`) y lanza la compilación en el clúster con Slurm. |

#### 🧪 Ejecución de Tests (JD Tests)

| Comando | Descripción |
| :--- | :--- |
| **`make run-jd`** | Sube el script `run-test-jd.sh` y lo lanza a la cola de Slurm (partición `stan`). |
| **`make tail-jd`** | Muestra en tiempo real el log de salida del test JD (`tail -f`). Usa `Ctrl+C` para salir. |
| **`make all-jd`** | Ejecuta `run-jd` e inmediatamente se pone a mostrar el log (`tail-jd`). |

#### 📥 Descarga de Resultados (Fetch)

| Comando | Descripción |
| :--- | :--- |
| **`make fetch-ppm`** | Descarga solo las imágenes generadas a la carpeta local `logs/img/`. |
| **`make fetch-txt`** | Descarga los logs de texto y reportes de Slurm a la carpeta local `logs/txt/`. |
| **`make fetch-all`** | Ejecuta ambos comandos anteriores (descarga todo). |

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
    make run-jd
    ```

3.  **Esperar resultados...** (Tómate un café mientras Slurm trabaja).

4.  **Descargar Datos:**
    ```bash
    make fetch-all
    ```

5.  **Analizar en Local:**
    Ahora tienes los archivos en `./logs/`.

---

## 📂 Estructura de Scripts

Para mantener el orden, no modifiques el `Makefile` directamente si no es necesario. Edita los scripts correspondientes:

*   **`scripts/deploy/sync.sh`**: Controla qué archivos se suben (y cuáles se ignoran como `.git`, `.password` `env.sh` o `build`).
*   **`scripts/remote/build.sh`**: Comandos que ejecuta Slurm para compilar (CMake settings).
*   **`scripts/remote/run-test-jd.sh`**: Script que ejecuta los casos de prueba.
*   **`scripts/generation/genTests.py`**: Script que crea los tests. Si quieres cambiar parámetros (más rayos, resolución, etc.), edita esto y corre `make gen-tests`.
*   **`scripts/analysis/`**: Scripts para comparar imágenes o generar gráficas en local.
