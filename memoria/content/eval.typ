// MÁXIMO 5 PÁGINAS -> SI ES NECESARIO EMPLEAR ANEXSOS PARA OTRAS GRÁFICAS Y CUESTIONES IMPORTANTES

= Evaluación rendimiento y energía <eval>
En esta sección realizaremos un análisis del rendimiento y de la energía, especialmente en el proceso de paralelización. Esta se analizará en base a diferentes factores, entre ellos: el número de hilos, granularidad y tipo de _partitioners_ . De este modo, hemos realizado comprobaciones de los tiempos de ejecución y energía consumida para diferentes valores de las métricas mencionadas. 
//Los datos resultantes, además de encontrarse en el repositorio dentro del directorio: _./memoria/graphs_  !!! HAY QUE VER REALMENTE DONDE SE METEN Y COMO PORQUE LO CSV DAN PROBLEMAS LOL
//TODO: poned la carpeta
//, podemos encontrarlos en la hoja de cálculo que podemos encontrar en el siguiente link:
== Evaluación del punto de partida secuencial
En primer lugar, cevaluaremos el punto de partida secuencial de nuestro proyecto antes de comenzar el proceso de paralelización de forma breve. 
Esto, nos permitirá establecer las bases y poder realizar comprobaciones y 
== Proceso de paralelización 
Para evaluar nuestro código y conseguir el mayor rendimiento y eficiencia energética, hemos realizado una evaluación exhaustiva de las mejores combinaciones de las metricas indicadas en la introducción de la sección para cada fragmento paralelizado (que se ha detallado en la sección anterior).

Para ello, se ha definidio un proceso de evaluación fijo para todos los casos que se define a continuación:. 
  #set enum(numbering: "1", start: 1)
  + En primer lugar establecemos el número de hilos del que empleará el fragmento. Para ello, antes de probar valores a fuerza bruta, comprobamos valores críticos, como son: 1 hilo, 28 (completando un socket), 56(requiere que se utilicen ambos sockets), 112(número de hilos que admite el procesador de manera física) y 120 (nos permite comprobar el funcioanmiento con hilos físicos). Esto, nos permitira aobservar la tendencia de mejora en el rendimiento del código y poder elegir un rango sobre el que trabajar más reducido para nuestras pruebas. 
  + Con el rando determinado y sin particionador, comprobamos todos los valores de hilos del rango con un step 2. Por ejemplo, en el caso de que el rango que parece más prometedor sea de 56 a 112 hilos, ejecutamos con 56, 58, 60 etc. hilos. De este modo, podemos comprobar que valores presentan un mejor rendimiento y reducir aún más las pruebas que nos permitiran establecer el partitioner y el tamaño de grano. 
  + Una vez hemos establecido un rango de valores reducido para el número de hilos (de unos 6-8 valores), probamos todas las combinaciones de granularidad y particionadores en cada uno de los valores, pudiendo así establecer cual es la combinación óptima y también identifiacar ciertos patrones al respecto. 

Una vez se ha realizado este estudio para todos los fragmentos paralelizados, los combinamos para la versión final, que también se estudiará más adelante. 

== Evaluación del resultado final 