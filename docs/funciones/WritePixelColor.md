# Función `WritePixelColor`

Esta función toma el color acumulado para un único píxel, realiza el procesamiento final (promedio de muestras, corrección gamma y escalado) y escribe los valores RGB de 8 bits resultantes en la salida estándar (`std::cout`), listos para ser parte de un archivo de imagen PPM.

## Firma Propuesta

```cpp
void WritePixelColor(std::ostream& out, Color pixel_color, int samples_per_pixel, float gamma_value);
```

## Parámetros

*   **`out`** (`std::ostream&`): El flujo de salida donde se escribirán los datos del píxel (por ejemplo, `std::cout` o un `std::ofstream` a un archivo).
*   **`pixel_color`** (`Color`): Un objeto de tipo `Color` (ej. `Vec3`) que contiene la **suma** de los valores de color de todas las muestras para un único píxel. Se asume que cada componente (R, G, B) está en un rango de punto flotante.
*   **`samples_per_pixel`** (`int`): El número de muestras que se utilizaron para calcular el `pixel_color`. Este valor es necesario para promediar el color.
*   **`gamma_value`** (`float`): El valor del parámetro gamma para realizar la corrección.

## Parte del enunciado
"
Para cada rayo generado se calcula el color obtenido al proyectar el rayo sobre la escena. Este color se
obtiene como un valor RGB donde cada componente se encuentra en el espacio de los números reales [0,1].
Posteriormente, las contribuciones de todos los rayos que se corresponden con un mismo píxel se promedian,
siendo este valor promedio el valor que se asigna a dicho píxel.
Antes de generar el valor final en la escala de 0 a 255, se deben realizar dos operaciones finales:
1. Aplicación de corrección gamma. Cada valor de intensidad en el rango [0,1] se eleva al exponente 1/γ . Esta corrección mejora la calidad de la imagen.
2. Escalado de intensidad. Cada valor en el rango [0,1] obtenido después de la corrección gamma se
escala al intervalo discreto [0,255]. De esta manera al valor original 0 le corresponde un valor discreto
de 0 y al valor original 1 le corresponde un valor discreto de 255. Si es necesario, se trunca el valor
obtenido.
"
