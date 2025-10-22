
# Entrar en Avignon

### Activar la VPN:

Si no estás en la uni no ahce falta

## Copia esto para entrar:

```ssh -l a052XXXX avignon.lab.inf.uc3m.es``` y luego mete tu contraseña

### Crea:

Crea la carpeta `rtx/` en `/home/alumnos/a052XXXX` con `mkdir rtx` que es donde el script de cargado de archivos guardará el código

### Comprueba:

Si creaste `rtx/` antes recuerda hacer `rm -rf ./rtx/` si quieres subir de nuevo el código si no quieres que se junte con el viejo.

# Sube archivos a _Avignon_

### Ejecuta:
Ejecuta `scripts/to_avignon.sh <a052XXXX>` desde otra instancia de la terminal de Bash del contenedor o de Linux si no eres ni Nico ni el q escribe; que no sea la misma que la que tiene la sesión de _Avignon_ abierta.

### Descomprime:

Ejecuta los siguientes comandos para descomprimir el archivo.
``` 
cd rtx
tar -xzvf render-proyecto.tar.gz
```
Luego elimina la carpeta `out/` si está
```
rm -rf out
```

# Ejecuta archivos en _Avignon_
/*esta incompleto*/
borra la carpeta out/ !!!!!!!!!
Haz desde _Avignon_: `sbatch ./scripts/build.sh` y después `sbatch ./scripts/run.sh`

# Valgrind

valgrind --tool=cachegrind program

cg_annotate cachegrind.out.XXXXX --auto=yes

