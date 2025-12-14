// MÁXIMO 5 PÁGINAS -> SI ES NECESARIO EMPLEAR ANEXSOS PARA OTRAS GRÁFICAS Y CUESTIONES IMPORTANTES
= Evaluación rendimiento y energía <eval>
En esta sección realizaremos un análisis del rendimiento y de la energía en base a diferentes factores, entre ellos: el número de hilos, granularidad y tipo de _partitioners_, permitiendo la comprensión de las decisiones de paralelización tomadas. 

== Proceso de paralelización 
Para evaluar nuestro código y conseguir el mayor rendimiento y eficiencia energética, hemos realizado una evaluación exhaustiva de las mejores combinaciones de las metricas indicadas en la introducción de la sección para cada fragmento paralelizado (que se ha detallado en la sección anterior).

Para ello, se ha definidio un proceso de evaluación fijo para todos los casos que se define a continuación:. 
  #set enum(numbering: "1", start: 1)
  + En primer lugar establecemos el número de hilos del que empleará el fragmento. Para ello, antes de probar valores a fuerza bruta, comprobamos valores críticos en base a la arquitectura del nodo _stan_, como son: 1, 28, 56, 112 y 120 hilos. Esto, nos permitira observar la tendencia de mejora en el rendimiento del código y poder elegir un rango sobre el que trabajar para nuestras pruebas. 
  + Con el rango determinado y sin particionador, comprobamos todos los valores de hilos del rango más prometedor con un step 2. De este modo, podemos comprobar que valores presentan un mejor rendimiento y reducir aún más las pruebas que nos permitiran establecer el partitioner y el tamaño de grano. 
  + Una vez hemos establecido un rango de valores reducido para el número de hilos (de unos 6-8 valores), probamos todas las combinaciones de granularidad y particionadores en cada uno de los valores, pudiendo así establecer cual es la combinación óptima y también identifiacar patrones al respecto. 


== Motor de renderizado (rendering_engine.hpp)
Dado que el motor de renderizado se presentaba como el mayor cuello de botella en nuestro código, su análisis resultaba especialmente relevante para la optimización del código. 

Tras ejecutar la primera prueba mencionada con anterioridad, obtenemos los resultados presentados. 
#figure(
  grid(
    columns: 2,
    gutter: 0cm,
    
    figure(image("../img/graficas/tiempo_renderizado.png", width: 50%),
    caption: [Evaluación rendimiento en base valores clave]),
    figure(image("../img/graficas/energia_renderizado.png", width: 50%),
    caption: [Evaluación consumo energético en base valores clave]),
  ),
)

Como se puede ver, en este caso (y en todos los que se presentarán al menos en esta sección de evaluación) el consumo de energía y el rendimiento del código están verdaderamente relaccionados y presentan un comportamiento muy similar. 

Más alá de eso, podemos ver como hay muy poca diferencia en la optimización entre las ejecuciones de 112 y 120 hilos, lo que nos puede dar a entender que el número óptimo de hilos debe estar cerca de 112 o ligeramente antes. Por tanto, procederemos a evaluar el rango de número de hilos de 56 a 120 hilos. 

Por tanto, en las siguientes gráfica presentamos los resultados de rendimiento y energía de dicho rango, y podemos comprobar como los números de hilso que presentan mejores resultados se encuentran entre los 108 y los 114 hilos. 

#figure(
  grid(
    columns: 2,
    gutter: 0cm,
    
    figure(image("../img/graficas/tiempo_renderizado.png", width: 50%),
    caption: [Evaluación rendimiento en base valores clave]),
    figure(image("../img/graficas/energia_renderizado.png", width: 50%),
    caption: [Evaluación consumo energético en base valores clave]),
  ),
)

Esto tiene total sentido, ya que son valores que se encuentran próximos al límite de paralelización físico que presneta la arquitectura de nuestra máquina. 

Por ello, realizaremos nuestro estudio de particionadores y grano en base a la ejecución con 108, 110 y 112 hilos. En este caso, y dado que el comportamiento es muy similar tanto en tiempo de ejcución como en energía consumida, presentaremos solo las gráficas de rendimiento. No obstante, las de energía se pueden encontrar en @Anexo-1
#figure(
  grid(
    columns: 3,
    gutter: 0.5cm,
    
    figure(image("../img/graficas/rendimiento-108.png", width: 100%),
    caption: [Evaluación rendimiento con 108 hilos]),
    figure(image("../img/graficas/rendimiento-110.png", width: 100%),
    caption: [Evaluación rendimiento con 110 hilos]),
    figure(image("../img/graficas/rendimiento-112.png", width: 100%),
    caption: [Evaluación rendimiento en base valores clave]),
  ),
)
En estas gráficas hay cosas más significativas a observar y comentar. 
Se puede ver en todos los casos, y para todos los valores posibles de granularidad, que el partitioner que presenta peor granularidad es el estático.

Esto se puede atribuir a que el static_partitioner hace un reparto del tarbajo de tamaño fijo y uniforme, siendo ideal para cargas de trabajo balanceadas y predecibles. Esto no encaja bien con el renderizado de pixeles, dado que hay una gran variación en el tartamiento que requieren en base a si en el se produce una intersección de rayos o no o del tipo de figura que se presenta. Y es precisamente esto por lo el simple_partitioner funciona especialmentte bien para esta carga de trabajo, pues admite balanceo y adaptación para cargas de trabajo irregulares como es nuestro caso. 

En cuanto a la granularidad, también presentan uniformidad, pues todos presentan valores óptimos con tamaño de grano 3. Esto es especialmente importante, y más tratandose de un bloque en 2 dimensiones. Con un grano 3 se permite el tratado de unos 9 píxeles por bloque, siendo estos lo suficientemente pequeños como para que el hilo esté balanceado de manera adecuada pero lo suficientemente grande como para amortizar el _overhead_.Además permite una buena localidad espacial. 

Mirando a las gráficas, aunque se observa un comportamiento similar y unos valores cercanos, podemos observar que el valor óptimo se presenta con 112 hilos, tamaño de grano 3 y un particionador simple, con un tiempo de ejecución de 1,79998 segundos y un consumo energético de 469,5 J, lo que se traduce en un speedup de mayor a 22 respecto a la versión secuencial solo con la paralelización de este fragmento.


== Procesado de Imagen (image_par.cpp)

Esta sección analiza el impacto de la paralelización en la etapa final de generación de imagen. 

Para la evaluación de los efectos de la paralelización de la generación de la imagen, hemos seguido los mismos pasos seguidos en el caso anterior y comentados al comienzo de la sección. 

No osbtantem, cabe destacar que, a diferencia del motor de renderizado, el procesado de imagen es una tarea con baja intensidad computacional pero alto volumen de accesos a memoria. 

Tal y como se mencionó en la metodología a seguir, Se ejecutó un barrido de 1 a 120 hilos. El tiempo desciende de 38.36 s (1 hilo) a 37.73 s (120 hilos). El speedup se satura en 1.016x, validando la _Ley de Amdahl_ que establece que dado $P_("seq") approx 0.995$ (el renderizado secuencial consume ~99.5%), el speedup teórico máximo es de $S_("max") = 1 / P_("seq") approx 1.005x$. 

De este modo, concluimos que el mayor speedup se presenta cuando se utilizan 100 hilos para la paralelización, de este modo

En las gráficas que se muestran a contonuación se pueden observar los datos indicados:

#figure(
  grid(
    columns: 2,
    gutter: 1em,
    image("../img/graficas/Gráfica1-Escalabilidad_Validación_Amdahl.png", width: 100%),
    image("../img/graficas/Gráfica2-La_Curva_de_Speedup.png", width: 100%),
  ),
  caption: [Izquierda: Tiempo de ejecución total vs hilos. Derecha: Curva de Speedup],
) <fig:image-scalability>

=== Eficiencia Energética y Optimización

#figure(
  grid(
    columns: (1fr, 1fr),
    gutter: 1em,
    image("../img/graficas/Gráfica3-Consumo_Energético.png", width: 100%),
    image("../img/graficas/Gráfica4-Barrido_de_Optimización.png", width: 100%),
  ),
  caption: [Izquierda: Consumo energético total vs hilos. Derecha: Comparativa de partitioners],
) <fig:image-energy>

El consumo energético muestra tendencia ascendente: al no reducirse el tiempo, más hilos incrementan la potencia media resultando en penalización energética. La comparativa de estrategias TBB (_auto, simple, static, affinity_) muestra homogeneidad indicando robustez: diferencias < 0.2s (ruido estadístico). El análisis identificó a #strong[`simple_partitioner`] como marginalmente superior.

#figure(
  image("../img/graficas/Gráfica5-Detalle_de_Granularidad_Fina.png", width: 60%),
  caption: [Detalle de granularidad para `simple_partitioner`],
) <fig:image-grain-simple>

El comportamiento bajo diferentes granos muestra óptimo en: #strong[`simple_partitioner` con grano 64 (37.67s)]. Aunque la mejora vs otras configuraciones es ~7ms, esta combinación minimiza el compromiso entre balanceo de carga y _overhead_ de gestión.

=== Conclusión de la Sección

La paralelización es funcional y correcta. Sin embargo, su impacto global es nulo debido al dominio de la parte secuencial. Basándonos en los datos empíricos, se selecciona la configuración #strong[`simple` / `64`] para la versión final, aunque se destaca que el sistema es insensible a variaciones en los parámetros de paralelización.