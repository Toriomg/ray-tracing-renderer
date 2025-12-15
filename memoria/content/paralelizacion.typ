// MÁXIMO 3 PÁGINAS    MUY IMPORTANTE !!!!!!

= Paralelización
Tras la realización de la evaluación del apartado anterior, hemos conseguido una versión paralelizada del código de la primera entrega. 

Para obtener los resultados mencionados a continuación se ha hecho uso de la librería TBB (Threading Building Blocks) que se presentó en clase como la herramienta a utilizar para la práctica. 

== Código original.
En nuestro caso, hemos paralelizado el fragmento del código que se encargaba del renderizado de las imágenes, que se encuentra más concretamente en el fichero _rendering_engine.hpp_. 

El fichero mencionado se realiza el renderizado de la imagen. Para ello, se recibe la información de la escena tridimensional y se transforma la misma en la imagen en dos dimensiones que debemos representar. 
Así, recorremos pixel a pixel la imagen y calculamos las intersecciones de rayos de los mismos que nos permiten generar las imágenes. 

Este proceso, se lleva a cabo en un gran loop que recorre las dos dimensiones de la imagen (filas y columnas). Teniendo esto en cuenta, nos encontramos ante un bucle bidimensional con una carga de cálculo altísima para calcular los rayos (como ya vimos en la entrega anterior).

== Funciones y algoritmos empleados
De este modo, hemos empleado diferentes recursos de la biblioteca. 

En primer lugar, hemos utilizado un _parallel_for_. Este algoritmo, divide el bucle en fragmentos más pequeños que se ejecutan cada uno en un hilo. Para establecer el rango del bucle hemos utilizado un _blocked_range2d_. Hemos elegido este bloque precisamente inspirados por el trabajo de multiplicación de matrices de los laboratorios vistos en clase. En este caso, nos enfrentábamos a una información que puede ser comprendida como una matriz, en la que los elementos de la  matriz son los pixeles sobre los que operamos, la anchura de la imagen el número de columnas y la altura el número de filas. 

Además, hemos adaptado el algoritmo a los resultado empíricos que resultan en el enunciado anterior, con el que hemos podido establecer el número de hilos, el tamaño de grano y el tipo de particionador que funcionará en el bucle que hemos definido con anterioridad. 

Para ello, primero de todo hemos establecido el límite de hilos que permitimos usar. En nuestro caso, según las pruebas realizadas e número óptimo de hilos es 112. Por ello, establecemos dicho limite con el uso de _global_control_ que también pertenece a la librería TBB y que permite establecer el número de hilos que se usan en nuestra implementación paralela. 

También establecemos el tamaño de grano. Esto, define el número de elementos que se le proporcionan a cada hilo. En nuestro caso, al usar un _blocked_range2d_, el tamaño de grano va a determinar bloques de nxn elementos (suponiendo un tamaño de grano n). Este valor, se añade como parámetro de la función parallel for y, como se ha visto en el análisis del apartado anterior, toma un valor de 3, lo que indica que cada hilo recibe un bloque de 3x3 pixeles sobre el que operar. Este tamaño de grano, como ya se ha mencionado antes, permite un buen funcionamiento del programa ya que no es demasiado pequeño como para hacer que la operación consuma menos recursos de los que se gastan en emplear un hilo nuevo, pero a su vez es lo suficientemente pequeño como para que la carga de cómputo esté balanceada y no haya mucha diferencia en el tiempo de ejecución de los distintos hilos. En este mismo area, también hemos empleado un _simple_partitioner_ que, como se ha visto en los laboratorios, permite la división hasta que llega al tamaño de grano asociado al rango. De este modo, el particionador refuerza el tamaño de grano y fomenta que se use la granularidad indicada y establecida. Este tipo de particionador funciona especialmente bien para este proceso concreto ya que cada pixel requiere una cantidad de trabajo muy similar, por lo que las cargas de trabajo están balanceadas y por tanto no necesitamos un _partitioner_ que gestione las cargas de manera "automática". 

== Cambios en el código. 
Empleando lo que se ha mencionado, el bucle de renderizado pasa de ser: 

*`
for (size_t row = 0; row < imageHeight; row++) {
    for (size_t col = 0; col < imageWidth; col++) {

      // LÓGICA DE INTERSECCIÓN DE RAYOS 
    }
  }

`*
A una lógica más compleja con el uso de la librería TBB, de modo que se emplean todas las funciones y herramientas que se han mencionado con anterioridad:

*`
tbb::global_control global_limit(tbb::global_control::max_allowed_parallelism, 112);

  tbb::parallel_for(
    tbb::blocked_range2d<size_t>(0, imageHeight, 3, 0, imageWidth, 3),
    [&](tbb::blocked_range2d<size_t> const & r) {

    // GENERADORES LOCALES

      for (size_t row = r.rows().begin(); row != r.rows().end(); ++row) {
        for (size_t col = r.cols().begin(); col != r.cols().end(); ++col) {

          // MISMA LÓGICA DE INTERSECCIÓN DE RAYOS 
        }
      }
    },

    tbb::simple_partitioner()
  );
      `*

Como se puede ver, la segunda implementación, que se corresponde con el código paralelizado, es más compleja y engorrosa, pues contiene todas las limitaciones que se han mencionado con anterioridad.

== Resultados 
Dado que, como ya se ha mencionado en el apartado anterior de la memoria, hemos establecido que esta paralelización es suficiente para obtener buenos resultados, pues la paralelización de otros fragmentos supondría una mejora prácticamente insignificantes del rendimiento y el consumo de la energía. 

Por ello, nuestro análisis de paralelización concluye con el renderizado de las imágenes, pues no hay más fragmentos del código que hayan requerido de su paralelización (pues las modificaciones en el generador de números aleatorios no se incluyen en este apartado si no en el de modificaciones presentado con anterioridad). 