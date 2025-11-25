#import "@preview/cetz:0.2.2"
= Diseño

Esta sección detalla el diseño de la aplicación, creado para soportar *las estrategias de datos SOA (_Structure of Arrays_) y AOS (_Array of Structures_)*, analizando su arquitectura, modelo conceptual y flujo de ejecución.

== Diseño de la estructura
El diseño estructural define los módulos principales del software, incluyendo sus responsabilidades y relaciones con otros módulos del proyecto. 

La arquitectura sigue un principio de separación de  responsabilidades. De tal manera que se aíslan las implementaciones específicas de SOA y AOS de la lógica común que es utilizada por ambas.

#figure(
  image("../img/arquitectura.png", width: 60%),
  caption: [Diagrama de componentes de la arquitectura.]
) <arquitectura>

Los componentes principales identificados en la figura @arquitectura son:

- *Módulo `common`*:Agrupa toda la lógica y las estructuras de datos que no dependen de la estrategia de almacenamiento de la imagen. Incluye:
  - Definiciones matemáticas (vectores, rayos, colores).
  - Lógica de intersección de rayos con objetos (esferas, cilindros).
  - Gestión de materiales y cálculo de reflexiones/refracciones.
  - El parser para los archivos de configuración y de escena.

- *Módulo `aos`*: Contiene la implementación específica para la estrategia
  *Array of Structures*. La principal diferencia reside en la estructura de
  datos que representa la imagen en memoria y el ejecutable `render-aos`.

- *Módulo `soa`*: Análogamente, contiene la implementación para la estrategia
  *Structure of Arrays*. Su estructura de imagen se compone de tres vectores
  separados para los canales R, G y B. También provee el ejecutable `render-soa`.

- *Ejecutables (`render-aos` y `render-soa`)*: Son los puntos de entrada de
  la aplicación. Su responsabilidad es gestionar los argumentos de la línea de
  comandos, orquestar el proceso de parseo y renderizado, y finalmente
  generar el archivo de salida en formato PPM.
== Diseño conceptual

El diseño conceptual se centra en las abstracciones y entidades clave que
modelan el problema del renderizado por trazado de rayos. Estas clases y
estructuras de datos forman el núcleo de la lógica de la aplicación.

#figure(
  // Inserta aquí tu diagrama de clases (UML o similar).
  // Debe mostrar las clases principales y sus relaciones (composición,
  // herencia, asociación).
  // Ejemplo: image("diagrams/modelo-dominio.png", width: 90%)
  rect(width: 100%, height: 5cm, stroke: black, inset: 1cm)[
    *Placeholder: Diagrama de Clases del Dominio*
  ],
  caption: [
    Modelo conceptual de las entidades principales del sistema.
  ]
) <modelo-dominio>

A continuación, se describen las responsabilidades de las abstracciones más
relevantes mostradas en la figura @modelo-dominio:

- *`Objeto`*: #lorem(20)



== Diseño de comportamiento
El diseño de comportamiento describe cómo los objetos y componentes
interactúan a lo largo del tiempo para llevar a cabo las tareas principales.
El flujo principal de la aplicación se puede dividir en dos fases: la carga de
la configuración y el bucle de _renderizado_.

#figure(
  // Inserta aquí tu diagrama de secuencia o actividad para el proceso de
  // renderizado principal.
  // Ejemplo: image("diagrams/flujo-renderizado.png", width: 70%)
  rect(width: 100%, height: 6cm, stroke: black, inset: 1cm)[
    *Placeholder: Diagrama de Secuencia del Bucle de Renderizado*
  ],
  caption: [
    Diagrama de secuencia que ilustra el proceso de renderizado para un único
    píxel.
  ]
) <flujo-renderizado>

El proceso completo, ilustrado en la figura @flujo-renderizado, sigue los
siguientes pasos:

#enum(
  start: 1,
  tight: false,
)[
  *Inicialización:* El programa principal (`main`) recibe las rutas de los
  archivos de configuración, escena y salida. Se instancia un `Parser` para
  leer y validar estos archivos, creando los objetos `Scene` y `Camera`.
]
#enum(
  start: 2,
  tight: false,
)[
  *Bucle principal de renderizado:* El programa itera sobre cada píxel de la
  imagen de salida (de arriba a abajo, de izquierda a derecha).
]
#enum(
  start: 3,
  tight: false,
)[
  *Muestreo por píxel (Anti-aliasing):* Para cada píxel, se inicia un bucle de
  muestreo. Se lanzan múltiples rayos (`samples_per_pixel`) a través de
  posiciones aleatorias dentro del área del píxel para suavizar los bordes.
]
#enum(
  start: 4,
  tight: false,
)[
  *Trazado de rayo:* Para cada rayo muestreado, se invoca a una función
  recursiva `ray_color()`. Esta función:
  - Calcula la intersección más cercana del rayo con los objetos de la `Scene`.
  - Si no hay intersección, devuelve el color de fondo.
  - Si hay intersección, consulta el `Material` del objeto golpeado para
    determinar cómo el rayo es atenuado y si es reflejado o refractado.
  - Se genera un nuevo rayo y se llama recursivamente a `ray_color()` con una
    profundidad reducida. El proceso se detiene al alcanzar la profundidad
    máxima (`max_depth`).
]
#enum(
  start: 5,
  tight: false,
)[
  *Acumulación y promedio:* El color resultante de cada rayo muestreado se
  acumula. Al final del bucle de muestreo, el color total se divide por el
  número de muestras para obtener el color final del píxel.
]
#enum(
  start: 6,
  tight: false,
)[
  *Corrección y almacenamiento:* Se aplica la corrección gamma al color
  promediado. El valor de color final (en formato de punto flotante `[0,1]`) se
  escala al rango de enteros `[0, 255]` y se almacena en la estructura de
  datos de la `Image` (ya sea en formato AOS o SOA).
]
#enum(
  start: 7,
  tight: false,
)[
  *Generación del archivo de salida:* Una vez procesados todos los píxeles,
  el contenido de la estructura `Image` se escribe en el archivo de salida
  siguiendo el formato PPM (P3).
]
