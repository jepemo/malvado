# Referencia de la familia DIV

Consulta: 2026-10-29. Recomendación de diseño: **BennuGD2 como referencia moderna principal**, con BennuGD clásico como inventario histórico y DIV/Fénix como referencias de semántica. No son APIs intercambiables ni un único estándar.

## Fuentes y elección

| Referencia | Fuente primaria | Uso propuesto |
| --- | --- | --- |
| DIV Games Studio | [Código, ayuda y ejemplos](https://github.com/DIVGAMES/DIV-Games-Studio), revisión consultada `77556912335d0a1b0efea855d4eb2490cf382128` | Modelo original de procesos y formatos, contraste histórico; ayuda en `help/`, runtime en `src/` |
| Fénix | [Distribuciones del proyecto](https://sourceforge.net/projects/fenix/files/Fenix/) | Comprobar diferencias de época; la página consultada lista 0.92a. No se ha fijado aquí un snapshot completo de Fénix 0.93 |
| BennuGD clásico | [Repositorio mantenido por SplinterGU](https://github.com/SplinterGU/BennuGD), revisión `d49f71891f38db9bbf08c0f6f0002007f564a03c` | 45 cabeceras de exportación de módulos: lista histórica muy útil y modular |
| BennuGD2 | [Repositorio de SplinterGU](https://github.com/SplinterGU/BennuGD2), revisión `baf8dc1b744d47dae58fc20c139dd44656907911` | 10 cabeceras de exportación; referencia principal para API de juegos moderna |
| Documentación BennuGD | [Portal del proyecto](https://divhub.github.io/bennugd-website/docs/) y [manual histórico](https://www.bennugd.org/downloads/static_docs/General.pdf) | Explicaciones y ejemplos; comprobar siempre contra revisión fuente |
| PixTudio y otras variantes | [DIV Project Manager](https://divpm.divhub.org/), que documenta soporte de varios runtimes | Candidatos a contraste futuro; no se ha capturado su API y no condicionan el contrato inicial |

La elección de BennuGD2 es una conclusión de diseño a partir de sus módulos y código; no una garantía de compatibilidad ni una afirmación de que cubra todas las funciones de DIV. Por ejemplo, BennuGD clásico tiene `mod_m7`, `mod_path`, `mod_flic`, `mod_cd`, `mod_blendop` y `mod_effects`; la organización moderna concentra muchas funciones en `libmod_gfx`, `libmod_misc`, `libmod_input` y `libmod_sound` e incluye módulos como red, ads e IAP. Hay que comparar nombres **y semántica**, no equiparar módulos por número.

## Qué se ha conseguido

Se pueden descargar las 55 cabeceras `*_exports.h` encontradas en los árboles completos de estas dos revisiones. Incluyen registros `FUNC`, constantes, cadenas de globales y locales, alias y ramas de preprocesador. Hay **492 declaraciones de funciones/sobrecargas en BennuGD clásico y 616 en BennuGD2**; no son 1.108 funciones únicas ni funciones de Malvado. El [generador](../tools/catalog_api.py) reproduce el [inventario](inventory.md) y el [catálogo](catalog.json) sin red después de descargar las fuentes.

También se pueden descargar cabeceras auxiliares y ficheros de referencia del runtime. Cada original tiene URL fijada y SHA-256 en el [manifest](upstream-manifest.json). No se incorpora ese código externo al build del motor.

## Límites de «toda la API»

El snapshot cubre todas las tablas de exportación de módulos de las revisiones citadas, comprobadas contra sus árboles. **No es todavía una especificación semántica completa** de DIV/Fénix/BennuGD y no certifica cada build: hay funciones condicionales, macros y alias; también construcciones del lenguaje, campos del core y extensiones externas que no se describen enteramente en esas tablas.

Queda trabajo explícito:

1. Catalogar sintaxis/semántica de `PROCESS`, `FRAME`, tipos y estructuras; el juego seguirá usando Lua, no un compilador de `.prg`.
2. Extraer campos de instancia del core (`id`, familia, prioridad, estado), variables del runtime y constantes no exportadas por módulos.
3. Fijar una versión Fénix concreta y auditar las funciones originales de DIV; no dar por cubierta su unión por tener BennuGD.
4. Para cada sobrecarga: nombres de argumentos, unidades, rangos, defaults, errores, ownership, efectos persistentes y plataformas.
5. Clasificar equivalentes, renombradas, alternativas y ausentes; verificar comportamiento leyendo implementación y ejecutando ejemplos de referencia.
6. Auditar extensiones de terceros por separado. Publicidad/IAP requieren servicios y SDK propios; no los proporciona raylib.

## Precauciones concretas frente a la [lista histórica de API](../history/legacy-api.md)

- `GET_ID` exporta una firma `I`: no asumir que `get_id()` sin argumentos significa «mi id». Podría ser un recorrido por tipo; hay que revisar implementación.
- `SIGNAL` exporta `II`; una entrada `signal(pid)` no documenta la señal ni sus efectos.
- `SET_FPS`, `WRITE`, `MAP_*`, colores y sonido tienen sobrecargas y diferencias entre generaciones.
- En Malvado antiguo `angle` usa grados y `size` usa 1.0; el inventario Bennu clásico declara `size=100` y `PI=180000` en su contexto de ángulos. Adaptar unidades deliberadamente.
- `write_var` con punteros a variables no se traduce pasando una dirección de Lua. Proponer un valor actualizable, getter Lua o binding específico, con costes claros.

## Cierre de la investigación por símbolo

Una ficha pasa de «inventariado» a «especificado» solo con referencia fijada, firma semántica, ejemplo, divergencias y prueba de conformidad prevista. Solo pasa a «implementado» después del código y las comprobaciones. El porcentaje de cobertura se calcula sobre el perfil y la revisión fijados, contando sobrecargas por separado y mostrando exclusiones.
