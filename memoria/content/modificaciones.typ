#import "@preview/cetz:0.2.2"

= Modificaciones de diseño 
El diseño inicial planteado en la primera práctica, pese a ser ya bastante sólido, presentaba algunas áreas a modificar de cara a la paralelización. 
Por ello, antes de comenzar a realizar la paralelización del código, se han realizado las siguientes modificaciones de diseño. 

== Implementación BVH. 
Ya planteamos una implementación AABB en la primera práctica. Sin embargo, dada que la complejidad de los escenarios a evaluar ha aumentado, 
hemos decidido implementar una estructura BVH (Bounding Volume Hierarchy) que nos permite optimizar aún más las intersecciones de rayos y objetos. 
La estructura BVH consiste en una jerarquía de volúmenes que agrupan los elementos de la escena, tratandolos como un arbol binario en el que cada 
caja ya definida en AABB se incluye en otra caja mayor que engloba varias de ellas, de este modo, podemos descartar grandes proporciones de los objetos, pues si no hay intersección con la caja mayor, no es necesario comprobar las cajas interiores.

Dichos cambios, se pueden encontrar en los ficheros _common/include/dataStructs/bvh.hpp_ y _common/src/dataStructs/bvh.cpp_.

Además de todo lo anterior, se han realizado otras modificaciones relativas a la implementación de la estructura BVH. 
Entre ellas, se incluye la ampliación del código relativo a AABB. Más especificamente, se han añadido funciones, que permiten la unión de dos cajas AABB, calcular el 
centroide (punto medio) de una caja AABB y calcular el eje más largo.

Por otro lado, tanto el renderizador como el parseador han sufrido modificaciones, aunque menores, para adaptarse a esta nueva estructura. 
En el _parser_ de la escena, simplemente se ha añadido una llamada a la clase BVH, de modo que se genere de el árbol BVH con el que se trabajará para 
el renderizado y que resultará en una gran eficiencia de cálculo de itersecciones. 

Por su parte, el renderizado ha sufrido modificaciones mucho más agresivas, requiriendo prácticamente una reescritura completa. Esto se debe a que el renderizador 
ya no renderiza como tal si no que es el encargado de contar el número de intersecciones que se producen en cada rayo. 

//TODO: Explicar un poco más los cambios del renderizador. 

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

//TODO: Explicar un poco más los scripts realizados y su funcionamiento de manera super breve, 1 o dos párrafos de 5 lineas, algo verdaderamente breve.

== Diseño final del proyecto. 
Tras las modificaciones especificadas, los esquemas y diagramas de clases del proyecto, han sido modificados y resultan tal y como se presenta a continuación. 

//TODO: Incluir diagramas actualizados y las explicaciones pertinentes. 