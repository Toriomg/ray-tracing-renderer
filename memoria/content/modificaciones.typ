#import "@preview/cetz:0.2.2"

= Modificaciones de diseño 
El diseño inicial planteado en la primera práctica, pese a ser ya bastante sólido, presentaba algunas áreas a modificar de cara a la paralelización. 
Por ello, antes de comenzar a realizar la paralelización del código, se han realizado las siguientes modificaciones de diseño. 

== Implementación del BVH. 
Ya planteamos una implementación con _Axis Aligned Bounding Boxes_ (AABB) en la primera práctica. Sin embargo, dada que la complejidad de los escenarios a evaluar ha aumentado, 
hemos decidido implementar una estructura _Bounding Volume Hierarchy_ (BVH) que nos permite optimizar aún más las intersecciones de rayos y objetos. 

La causa de haber implementado este sistema ahora y no en la entrega anterior, se debe al coste inicial de búsqueda de la intersección entre rayos y objetos. Esto provoca que escenarios con pocos objetos demoren de un mayor tiempo en encontrarla (\~10-20% más). Sin embargo, se logra que un gran mayor rendimiento temporal en el último caso (\~40-45% menos).

La estructura del BVH consiste en una jerarquía de volúmenes que agrupan los elementos de la escena, tratandolos como un arbol binario en el que cada 
caja ya definida en AABB se incluye en otra caja mayor que engloba varias de ellas, de este modo, podemos descartar grandes proporciones de los objetos, pues si no hay intersección con la caja mayor, no es necesario comprobar las cajas interiores. 

Dichos cambios, se pueden encontrar en los ficheros _common/include/dataStructs/bvh.hpp_ y _common/src/dataStructs/bvh.cpp_.

Además de todo lo anterior, se han realizado otras modificaciones relativas a la implementación de la estructura del BVH. 
Entre ellas, se incluye la ampliación del código relativo a la AABB. Más especificamente, se han añadido funciones, que permiten la unión de dos AABB, calcular el 
centroide (punto medio) de una AABB y calcular el eje más largo.

Por otro lado, tanto el renderizador como el parseador han sufrido modificaciones, aunque menores, para adaptarse a esta nueva estructura. 

En el _parser_ de la escena, simplemente se ha añadido una llamada a la clase BVH, de modo que se genere de el árbol BVH con el que se trabajará para 
el renderizado y que resultará en una gran eficiencia de cálculo de itersecciones. 

Por su parte, el renderizado ha sufrido modificaciones mucho más agresivas, casi reescribiendo el funcionamiento estructural del módulo. Con la implementación del BVH la consideración previa de renderizar objetos, se ha tenido que reimplementar a una bajo la detección de colisiones entre rayos y objetos.

De este modo, y con todas las modificacione realizadas, se ha permitido una mejora sustancial en el rendimiento de la generación de imagenes, especialmente
en aquellas escenas con un gran número de objetos y una complejidad mayor ( que se estudiará más adelante de manera mucho más profunda). 

//TODO: Si queda hueco y como última cosa a hacer, incluir algún diagrama del funcionamiento de BVH como árbol, cajas o lo que sea.

== Cambios en el randomizador. 
En la primera práctica, dado que la ejecución era secuencial, el randomizador empleado no contaba con ningún tipo de seguridad para threads, 
es decir, no era thread-safe. Por ello, y de cara a su uso en diferentes hilos, y para evitar posibles secuencias o errores en la generación 
de imagenes, hemos tenido que cambiar su diseño. 

Para ello, definimos las semillas de cada hilo como atómicas, de modo que estas sean únicas, no se repitan entre hilos y no generen ningun tipo de secuencia.
Además, para garantizar eficiencia y evitar la creación de más generadores aleatorios que hilos, hemos optado por definir los generadores 
como _enumerable_thread_specific_, de este modo, cada hilo cuenta con su propia instancia del generador aleatorio y evitamos posibles conflictos entre hilos. 

El código relativo a dicha implementación se encuentra en _common/include/utils/random_par.hpp_.

//TODO: Añadir gráficas del funcionamiento del randomizador en diferentes hilos.

== Scripts de automatización de pruebas.
Pese a no formar parte explicita del código a desarrollar durante la práctica, nos parecía relevante hacer una breve mención a los scripts de automatización 
de pruebas realizados a lo largo de la práctica, ya que estos han sido un factor determinante en la eficacia y rapidez del desarrollo de la práctica en su 
totalidad, permitiendo lanzar pruebas en Avignon de manra rápida, automática y muy práctica. 

Estos scripts permiten compilar, ejecutar casos de evaluación y descargar los 'logs' e imágenes resultantes. Esto se hace mediante _Makefile_ para la simplificación de comandos en la terminal local y _sshpass_ para automatizar la gestión del acceso a Avignon. 

La compilación se realiza gracias a un script de _Bash_ (_scripts/remote/build.sh_) que despliega el código en Avignon y lanza una _SLURM_ para la compilación en Avignon. La ejecución de casos (_scripts/remote/run-test-jd.sh_) se realiza lanzando un _sbatch_ en Avingon que ejecuta ese _script_ que se encarga de ejecutar con _perf_ cada uno de los casos y guarda la salida del programa. Otros _scripts_ permiten descargar los tiempos de ejecución y modificar los ficheros _.csv_ con los que creamos las tablas de tiempo de versiones de la memoria.

== Diseño final del proyecto. 
Tras las modificaciones especificadas, tan solo se han producido cambios en las entidades y estructuras de datos del programa. La estructura y comportamiento de este sigue igual, más allá de los cambios de paralelización. 

#figure(
  image("../img/esquemas/scena.drawio.png", width: 75%),
  caption: [Esquema conceptual nuevo del proyecto]
  )<Figura1>

En la @Figura1 se muestra el nuevo diseño del proyecto. Se ha incluido a _SceneSettings_ el atributo _bvh_, que es el BVH que tiene la estructura. Además se ha creado la clase _BVH_ y modificado _AABB_. El resto de estructuras que se muestran en el diagrama son envoltorios (_wrappers_) de datos para asegurar la baja aridad de las funciones, evitar la sobrecarga del código y realizar el principio de programación DIY (_Don't Repeat Yourself_). 