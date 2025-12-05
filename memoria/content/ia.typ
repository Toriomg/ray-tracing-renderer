= Uso de herramientas de IA
En el desarrollo de este proyecto, y conforme a las directrices establecidas en el enunciado y en los estándares establecidos 
en el departamento, tal y como ya se hizo en la primera entrega de la práctica, a lo largo de la realización del proyecto, hemos 
empleado diversas herramientas de Inteligencia Artificial como apoyo para la productividad, la organización y sobre todo, la 
resolución de errores y bugs en el código desarrollado.

A continuación, se detalla el rol específico que cada modelo ha desempeñado en nuestro flujo de trabajo, destacando, como es 
evidente, que la responsabilidad final sobre el diseño, implementación y verificación del código recae exclusivamente sobre 
nosotros, los integrantes del grupo.

Pese a que el uso de dichas herramientas ha variado en base al integrante del grupo, en términos generales, se han empleado 
las siguientes herramientas, que se mantien muy similares a las empleadas en la entrega anterior del proyecto: 

#enum(
  start: 1,
  tight: false,
)[
  #underline[*Gemini 2.5 Pro: Asistente de Desarrollo Integral:*]

  Gemini 2.5 Pro fue la herramienta más utilizada a lo largo del proyecto, principalmente por su actualización al comienzo del mismo, 
  cosa que ha icrementado notablemente su utilidad. Podemos dividir su uso en las siguientes areas principales:

  #list(
    [Validación de organizanización en la estructura del proyecto y sujerencias de mejora.],
    [Ayuda de diseño y _brainstorming_ de desarrollo del proyecto.],
    [Comprobaciones de código rápido y depurado de errores sencillos.]
)
  En diferencia a la entrega anterior, y pese a seguir sin ser la herramienta más útil para la resolución de errores comoplejos, la actualización
  ha favorecido mucho su uso a lo largo de la realización del proyeecto. Además, dada su velogidad de respuesta y agilidad resulta una herramienta 
  muy útil para un proyecto de este tipo, en el que no hay una cantidad abundante de código pero si de comprobaciones y modificaciones, que pueden
  dar lugar a pequeños errores de facil resolución, que Gemini 2.5 Pro es capaz de solventar de manera rápida y eficaz frente a otros LLM con 
  un tiempo de respuesta y "razonamiento" mucho más lento.
]
#enum(
  start: 2,
  tight: false,
)[
  #underline[*Deepseek: Especialista en Problemas Complejos:*]
  
  Recurrimos a Deepseek de manera notoriamente más selectiva, principalmente cuando nos enfrentábamos a complejos _bugs_ que resultaban incapaces 
  de solucionar con otros LLM. Su uso viene condicionado principalmente por la velocidad de respuesta de dicha herramienta. Su lentitud de 
  razonamiento lo hace muy eficaz para la resolución de problemas más complejos y el depurado de código, no obstante, la pérdida de agilidad 
  que supone limita su uso a casos aislados. 
  
  Es más, en esta entrega, dado que la cantidad de código a producir ha sido significativamente menor que en el caso de la entrega anterior, y 
  por tanto el número de bugs críticos también se ha visto reducido, el uso de Deepseek ha sido mucho más limitado, centrandose casi 
  exclusivamente en la resolución de problemas relaccionados con la implementación en BVH que no fuimos capaces de resolver con Gemini.
]

En conclusión, el uso de herramientas de IA ha sido un apoyo valioso en la realización de este proyecto, facilitando tanto la organización como 
la velocidad de realización de la misma. 