#import "uc3mreport.typ": conf

#show: conf.with(
  degree: "Grado en Ingeniería Informática",
  subject: "Arquitectura de Computadores",
  year: (25, 26),
  project: "Proyecto de programación paralela",
  title: "Aplicación de sintesis de imágenes 3D",
  group: 81,
  team: 10,
  authors: (
    (
      name: "Héctor",
      surname: "Molina Garde",
      nia: 100522253
    ),
    (
      name: "Noa",
      surname: "López Fernández",
      nia: 100522230
    ),
    (
      name: "Guillermo",
      surname: "González Avilés",
      nia: 100522146
    ),
    (
      name: "Nicolás",
      surname: "Maire Bravo",
      nia: 100522100
    ),
  ),
  professor: "José Daniel García Sánchez",
  toc: true,
  logo: "old",
  language: "es"
)

#include "content/modificaciones.typ"
#include "content/paralelizacion.typ"
#include "content/eval.typ"
#include "content/org.typ"
#include "content/concl.typ"
#include "content/ia.typ"
#include "content/bibliografia.typ"
#include "content/anexos.typ"
