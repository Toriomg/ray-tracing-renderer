= Uso de herramientas de IA
En el desarrollo de este proyecto, y conforme a las directrices establecidas en el enunciado y en los estándares establecidos en el departamento, a lo largo de la realización del proyecto, hemos empleado diversas herramientas de Inteligencia Artificial como apoyo para la productividad, la resolución de problemas y la consolidación de los conocimientos adquiridos. 

A continuación, se detalla el rol específico que cada modelo ha desempeñado en nuestro flujo de trabajo, destacando, como es evidente, que la responsabilidad final sobre el diseño, la corrección y el rendimiento del código ha recaído siempre sobre los integrantes del equipo. 

Pese a que el uso de dichas herramientas ha variado en base al integrante del grupo, en términos generales, se han empleado las siguientes herramientas.

#enum(
  start: 1,
  tight: false,
)[
  #underline[*Gemini 2.5 Pro: Asistente de Desarrollo Integral:*]

  Gemini 2.5 Pro fue la herramienta más utilizada a lo largo del proyecto con un uso principalmente concentrado en tres áreas clave para el desarrollo del proyecto:

  #list(
    [Consultoría de los primeros conceptos necesarios para la realización del proyecto, proporcionando ayuda en temas con los que no nos sentíamos muy familiarizados.],
    [Ayuda de diseño y _brainstorming_ de desarrollo del proyecto.],
    [Principal herramienta de desarrollo y comprobaciones de código rápido, dada su capacidad para grandes ventanas de contexto.]
)
  No obstante, en ocasiones su uso resultaba desesperante al no ser el mejor LLM para el depurado de código, y en casos de asistencia para solucionar _bugs_ podía resultar notoriamente inefectivo. Es más, tras consultar acerca de uno de los mayores errores que enfrentamos en la realización del código, la herramienta repetía en bucle las mismas ideas erróneas para solucionarlo, que habíamos comprobado previamente como no funcionales. No obstante, fuimos capaces de identificar el error tras muchos intentos, guiando a la herramienta hacia los casos que nosotros veíamos como más factibles.
]
#enum(
  start: 2,
  tight: false,
)[
  #underline[*Deepseek: Especialista en Problemas Complejos:*]
  
  Recurrimos a Deepseek de manera notoriamente más selectiva, principalmente cuando nos enfrentábamos a complejos _bugs_ que resultaban incapaces de solucionar con otros LLM. Su uso viene condicionado principalmente por la velocidad de respuesta de dicha herramienta. Su lentitud de razonamiento lo hace muy eficaz para la resolución de problemas más complejos y el depurado de código, no obstante, la pérdida de agilidad que supone limita su uso a casos aislados. 
  
  Un símil que solemos usar en el equipo para explicar dichas diferencias con respecto a los otros LLMs utilizados durante el proyecto es el de un francotirador: certero y preciso (Deepseek), pero sin la cadencia, comodidad ni agilidad de una metralleta (Gemini).
]
#enum(
  start: 3,
  tight: false,
)[

  #underline[*ChatGPT 5: Tutor Conceptual y Clarificador Ágil de Código:*]

  El rol de ChatGPT 5 fue mayoritariamente pedagógico y lo empleamos como un tutor para afianzar nuestra comprensión de conceptos teóricos concretos y como un asistente que permitía analizar pequeños fragmentos de código ya escritos. 
  
  En concreto, este LLM es muy veloz, no osbtante, con el plan de uso gratuito, el uso esta muy limitado tanto en numero de prompts, longitud de mensajes y capacidad para proporcionar contexto. Por ello, aunque ha sido usado en momentos puntuales, su uso para el desarrollo o revisión de código ha sido prácticamente nulo y su uso se ha reducido más a un contexto explicativo.
 ]