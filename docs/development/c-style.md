# Estilo del código C

Escribir código sencillo para que otra persona pueda entenderlo, modificarlo y depurarlo. Priorizar claridad y contratos explícitos sobre abstracciones preventivas o trucos de lenguaje.

- Funciones pequeñas con una responsabilidad clara; nombres concretos y consistentes con su ámbito. Mantener juntos los datos que comparten ciclo de vida.
- Una operación importante por línea; evitar funciones enteras comprimidas, macros complejas y anidamiento innecesario. Retornar pronto ante errores.
- Comentar invariantes, ownership y decisiones que no sean evidentes. Mantener los comentarios sincronizados con el comportamiento.
- Hacer explícitos creación, destrucción, estados y errores; comprobar tamaños, límites y fallos de asignación. No ocultar efectos globales tras nombres ambiguos.
- Incluir en cada archivo propio una cabecera con atribución e identificador `SPDX-License-Identifier: Apache-2.0` para código propio de Malvado 2; consultar la [licencia](licensing.md); mantener intactos los avisos de terceros.
- Cabeceras con interfaz mínima; dependencias y detalles internos en archivos `.c`. Separar el código portátil de APIs de plataforma cuando tenga una responsabilidad real.
- Seguir el formato local coherentemente: indentación de cuatro espacios, llaves legibles, líneas de longitud razonable y espacios entre funciones. No reformatear código de terceros en `vendor/`.
- Las pruebas deben describir el comportamiento observable, especialmente errores, bordes y ciclos de vida, sin duplicar la implementación.

El runtime de alpha.1 todavía tiene estado global y reúne scheduler, binding y dibujo: la separación prevista en el roadmap sigue pendiente. Este estilo orienta nuevas contribuciones y la refactorización gradual, sin afirmar que el código actual ya cumple todos los puntos.
