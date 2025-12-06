#import "@preview/cetz:0.2.2"

= Modificaciones de diseño 
Pese a la solidez del diseño inicial, este presentaba algunas áreas a modificar de cara a la paralelización del código. 
Por ello, antes de comenzar a paralelizar, se han realizado ciertas modificaciones de diseño. 

== Implementación de BVH. 
Ya planteamos una implementación con _Axis Aligned Bounding Boxes_ (AABB) en la primera práctica. Sin embargo, dada que la complejidad de los escenarios a evaluar ha aumentado, hemos decidido implementar una estructura _Bounding Volume Hierarchy_ (BVH) que nos permite optimizar aún más las intersecciones de rayos y objetos. 
Su implementación ahora y no en la anterior entrega se debe al coste inicial de búsqueda de la intersección entre rayos y objetos, provocando que escenarios con pocos objetos demoren de un mayor tiempo en encontrarla (\~10-20% más), pero logrando un mayor rendimiento en casos con gran cantidad de objetos como la última escena (\~40-45% menos).

La estructura del BVH consiste en una jerarquía de volúmenes que agrupan los elementos de la escena, tratandolos como un arbol binario en el que cada 
caja ya definida en AABB se incluye en otra caja mayor que engloba varias de ellas, de este modo, podemos descartar el cálculod de intersección de grandes cantidades de objetos, pues si no hay intersección con la caja mayor, no es necesario comprobar las cajas interiores. 

Además, se han realizado otras modificaciones relativas a la implementación de la estructura del BVH. 
Por ejemplo, la ampliación del código relativo a la AABB, añadiendo funciones que permiten la unión de dos AABB, calcular el centroide de una AABB y su eje más largo.

Por otro lado, tanto el renderizador como el parseador han sufrido modificaciones, aunque menores, para adaptarse a esta nueva estructura.
En el _parser_ de la escena, simplemente se ha añadido una llamada a la clase BVH, de modo que se genere de el árbol BVH con el que se trabajará para el renderizado.
Por su parte, el renderizado ha sufrido modificaciones más agresivas, casi reescribiendo el funcionamiento estructural del módulo. Con la implementación del BVH se ha tenido que reimplementar el renderizado bajo la detección de colisiones entre rayos y objetos.


De este modo, y con todas las modificacione realizadas, se ha permitido una mejora sustancial en el rendimiento de la generación de imagenes, especialmente en aquellas con mayor complejidad. 

//TODO: Si queda hueco y como última cosa a hacer, incluir algún diagrama del funcionamiento de BVH como árbol, cajas o lo que sea.

== Cambios en el randomizador. 
En la primera práctica, dado que la ejecución era secuencial, el randomizador empleado no contaba con ningún tipo de seguridad para hilos. Por ello, y de cara a su uso en paralelo, evitando posibles secuencias o errores en la generación de imagenes, hemos tenido que reescribirlo.

En este caso, definimos las semillas de cada hilo como atómicas, haciendo que no se repitan entre hilos y no generen ningun tipo de secuencia o patrón.
Además, para garantizar eficiencia y evitar la creación de más generadores aleatorios que hilos, hemos optado por definir los generadores como _enumerable_thread_specific_. De este modo, cada hilo cuenta con su propia instancia del generador aleatorio y evitamos posibles conflictos entre hilos. 

== Scripts de automatización de pruebas.
Pese a no formar parte explicita del código a desarrollar durante la práctica, nos parecía relevante hacer una breve mención a los scripts de automatización 
de pruebas realizados, ya que estos han sido un factor determinante en la eficacia y rapidez del desarrollo de la práctica en su totalidad. 

Estos scripts permiten compilar, ejecutar casos de evaluación y descargar los 'logs' e imágenes resultantes mediante _Makefile_ para la simplificación de comandos en la terminal local y _sshpass_ para automatizar la gestión del acceso a Avignon. 

La compilación se realiza gracias a un script de _Bash_ que despliega el código en Avignon y lanza una _SLURM_ para la compilación. La ejecución se realiza mediante un _sbatch_ en Avingon que ejecuta un _script_ que se encarga de ejecutar con _perf_ cada uno de los casos y guardar la salida del programa. Otros _scripts_ permiten descargar los tiempos de ejecución y modificar los ficheros _.csv_ con los que creamos las tablas de tiempo de versiones de la memoria.

== Diseño final del proyecto. 
Tras las modificaciones especificadas, tan solo se han producido cambios en las entidades y estructuras de datos del programa. La estructura y comportamiento de este sigue igual, más allá de los cambios de paralelización. 

#figure(
  image("../img/esquemas/scena.drawio.png", width: 70%),
  caption: [Esquema conceptual nuevo del proyecto]
  )<Figura1>

En la @Figura1 se muestra el nuevo diseño del proyecto. Se ha incluido a _SceneSettings_ el atributo _bvh_. Además se ha creado la clase _BVH_ y modificado _AABB_. El resto de estructuras que se muestran en el diagrama son envoltorios (_wrappers_) de datos para asegurar la baja aridad de las funciones, evitar la sobrecarga del código y realizar el principio de programación DIY (_Don't Repeat Yourself_). 