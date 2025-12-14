= Organización de trabajo
Dado que en la entrega previa del proyecto la organización del trabajo fue verdaderamente eficaz y nos permitió entregar el trabajo a tiempo y con unos 
buenos resultados, hemos decididio utilizar de nuevo el método de organización por sprints para la realización de esta segunda parte, empleando 
herramientes de desarroyo Agile como Jira, que nos permiten saber en todo momento las tareas pendientes, pudiendo asi, tener una visión completa 
del estado y evolución del trabajo. 


En este caso, dado que tanto la cantidad de código a producir como el tiempo de trabajo ha sido significativamenete menor, los sprints son de 
menor duración y cantidad de tareas, no obstante, se han mantenido cuatro fases de trabajo como en la entrega previa y se han incluido a la 
planificación las tareas esenciales a realiar en cada sprint desde el primer momento. 

En la siguiente imagen, podemos comprobar cual ha sido la planificación inicial de sprints en un calendario, representando cada color una etapa diferente
de trabajo en el calendaro desde la publicación del proyecto hasta la entrega del mismo. 
#figure(
  image("../img/sprints/calendario-sprints.png", width: 100%),
  caption: [Calendario de sprints para la entrega]
  )

== División de trabajo
La subdivisión en secciones de trabajo y sus correspondientes sprints se incluyen a continuación:
#enum(
  start: 1,
  tight: false,
)[
  *Organización y diseño:* 
  En esta etapa, se han ralizado la creación y modificación del repositiorio de modo que se pueda paralelizar adecuadamente. 
  Esto incluye la modificación del randomizador y modificaciones como la inclusión del modelo BVH. 
  
  Esta etapa es realmente importante dado que nos permite establecer una base sólida para el proyecto. 
  Por ello, como se puede ver en el calendario de sprints presentado,su duración es mayor, pues un buen trabajo en el mismo nos permitirá encontrar menos contratiempos y errores durante el resto de la práctica.

  #figure(
  image("../img/sprints/sprint1.png", width: 100%),
  caption: [Sprint 1 de tareas: Organización y diseño.]
  )
]
#enum(
  start: 2,
  tight: false,
)[
  *Estudio de paralelización:* 
  Una vez sabemos cual es nuestro punto de partida, hemos dedicado cierto tiempo a establecer y estudiar de manera 
  más teórica posibilidades de paralelización e incluso comprobando algunas de ellas. 
  
  Esto nos permite establecer los cuellos de botella 
  de nuestro código y así poder enforcar nuestras energias en fragmentos concretos del código que nos van a ofrecer una mayor mejora en el código. 
  #figure(
  image("../img/sprints/sprint2.png", width: 100%),
  caption: [Sprint 2 de tareas: Estudio de paralelización]
  )
]
#enum(
  start: 3,
  tight: false,
)[
  *Búsqueda de solución óptima:*:
  Una vez hemos determinado los cuellos de botella de nuestro código en el sprint anterior, utilizamos esta etapa para realizar pruebas de granularidad, numero de hilos etc. de modo que podamos exprimir al máximo la eficiencia de los fragmentos paralelizados, consiguiendo los mejores tiempos posibles. 
  #figure(
  image("../img/sprints/sprint3.png", width: 100%),
  caption: [Sprint 3 de tareas: Búsqueda de solución óptima.]
  )
]
#enum(
  start: 4,
  tight: false,
)[
  *Memoría y últimos detalles:* 
  Para finalizar, una vez tenemos las mejores combinaciones de paralelización de cada fragmento paralelizado, juntamos dichas combinaciones establecemos unos tiempos de ejecución finales y pasamos toda la documentación correspondiente a la memoria para su enntrega.
  #figure(
  image("../img/sprints/sprint4.png", width: 100%),
  caption: [Sprint 4 de tareas: Memoria y últimos detalles.]
  )
]
== Desarrollo de tareas
Es importante destacar que las tareas son orientativas y se preparan al comienzo del trabajo y por tanto pueden sufrir modificaciones 
en el momento de su realización o es posible que se realicen tareas que ocurran de imprevisto y no estén incluidas en dichos sprints.

Por ello, además de los sprints presentados en la sección anterior que presentan las principales tareas a realizar de manera general, la subdivisión en subtareas dentro de dichos sprints, hemos generado una hoja de cálculo en la que se especifican detalladamente la realización de las diferentes subtareas por los diferentes miembros del grupo agrupadas por sprint, así como las horas dedicadas a cada una y la fecha de realización. Dicha hoja de cálculo se puede encontrar en el siguiente link:
#link("https://docs.google.com/spreadsheets/d/1CjTiw-uDLS2Pg-pdDF0c5OVARUC9LJG7lg4HX2hukao/edit?usp=sharin")[Organización de trabajo del equipo 10]
