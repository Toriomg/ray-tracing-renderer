= Organización de trabajo
== División de trabajo
Para garantizar la entrega a tiempo del proyecto en cuestión, primero se ha desarrollado una estructura lógica del tiempo disponible y dividido el desarrollo del proyecto en 4 secciones de trabajo que se han organizado de manera que bien podría recordar a un proyecto de Desarrollo Agile, en el que se subdividen etapas y se realizan entregas continuas de pequeñas tareas hasta completar dichas secciones. Para ello, hemos empleado la herramienta *Jira*, que ha permitido la organización en *Sprints* (pequeñas tareas) de las diferentes secciones de trabajo. 

La subdivisión en secciones de trabajo y sus correspondientes sprints se incluyen a continuación:
#enum(
  start: 1,
  tight: false,
)[
  *Diseño:* Durante la etapa, se realizó una investigación activa y concreta de recursos, modelos y conceptos que se usan en el ámbito del renderizado de imágenes y posibles formas de optimización del mismo que pudieran resultar eficientes para la tarea concreta. También se desarrollo el diseño general del código a realizar. Notese como en este sección no se programó en absoluto, simplemente organizamos los pasos a seguir para una programación efectiva y rápida.
  #figure(
  image("../img/sprint1.png", width: 100%),
  caption: [Sprint 1 de tareas: Diseño.]
  )
]
#enum(
  start: 2,
  tight: false,
)[
  *Desarrollo funcional*: En esta etapa, la misión principal fue asegurar que conseguíamos representar esferas y cilindros,sus materiales y conseguíamos generar imágenes .ppm de manera correcta. El objetivo prinicipal de la sección no era tanto conseguir elementos funcionales, optimizados y eficientes, si no garantizar la  validez del diseño realizado y la generación de un producto mínimo para poder aplicar las optimizaciones, o lo que es lo mismo, garantizar que al menos se pueden generar las imágenes y elementos correctamente. 
  #figure(
  image("../img/sprint2.png", width: 100%),
  caption: [Sprint 2 de tareas: Desarrollo.]
  )
]
#enum(
  start: 3,
  tight: false,
)[
  *Optimización y pruebas*: Esta sección resulta de las más relevantes del proyecto. En ella, se trabaja sobre lo obtenido en la sección anterior para optimizar los resultados, garantizando la eficiencia energética y la optimización del tiempo de ejecución. Para ello, se utilizan las técnicas descubiertas durante la fase de diseño y se aplican al código desarrollado durante la fase de desarrollo. En esta fase también se realizan las pruebas del programa, tanto unitarias como de rendimiento y energía. En definitiva, se garantiza que el código desarrollado es eficiente, correcto y cumple con los estándares mínimos de calidad y eficiencia requeridos.
  #figure(
  image("../img/sprint3.png", width: 100%),
  caption: [Sprint 3 de tareas: Optimización y pruebas.]
  )
]
#enum(
  start: 4,
  tight: false,
)[
  *Últimos detalles y evaluación del rendimiento*: Una vez realizado todo el código y solucionados los problemas y bugs hasta el momento, está última sección de trabajo se centró principalmente en realizar la correspondiente evaluación de rendimiento y energía, asegurar el correcto funcionamiento de los tests unitarios y funcionales y refactorizar completamente el código para asegurar el correcto cumplimiento de las restricciones de _clang tidy_. 
  #figure(
  image("../img/sprint4.png", width: 100%),
  caption: [Sprint 4 de tareas: Últimos detalles y evaluación del rendimiento.]
  )
]
== Desarrollo de tareas
Además de los sprints presentados en la sección anterior que presentan las principales tareas a realizar de manera general, la subdivisión en subtareas dentro de dichos sprints, hemos generado una hoja de cálculo en la que se especifican detalladamente la realización de las diferentes subtareas por los diferentes miembros del grupo agrupadas por sprint, así como las horas dedicadas a cada una y la fecha de realización. Dicha hoja de cálculo se puede encontrar en el siguiente link:
#link("https://docs.google.com/spreadsheets/d/1rZvKKSUwDupttnVyCiZqNnb4da4262B1Z6Bu74ui1zQ/edit?usp=sharing")[Organización de trabajo del equipo 10]
