# Documentación de las APIs C y Lua

Estado: decisión de planificación, no sistema implementado. Fecha de consulta: 2026-10-29. Las dos APIs son de primera clase y deben tener referencia, ejemplos ejecutables y guía de inicio. El [núcleo C y el binding Lua oficial](../architecture/c-first.md) se documentan con los mismos comportamientos y sus diferencias de lenguaje.

## Recomendación: Markdown + MkDocs + GitHub Pages

1. **Markdown en Git** como documentación legible en GitHub y revisada con el código.
2. **MkDocs con tema por defecto**, fijado en las dependencias de documentación, para navegación, búsqueda y publicación de un único sitio. MkDocs parte de archivos Markdown y una configuración `mkdocs.yml`; no hace falta un frontend propio. [Documentación oficial](https://www.mkdocs.org/user-guide/writing-your-docs/).
3. **GitHub Actions y Pages** para construir el sitio en cada PR y desplegar desde la rama o tag autorizado. [GitHub Pages](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages) admite workflows de generadores estáticos.
4. **Fuentes de la API:** declaraciones y comentarios de las cabeceras públicas C; definiciones Lua anotadas para el binding registrado desde C; páginas Markdown por módulo con contratos y ejemplos en ambos lenguajes.

Esta opción necesita un solo generador de sitio y no exige desde el inicio un esquema propio, plugins de documentación de C, ni generar HTML distinto para cada lenguaje. El contenido importante sigue siendo legible directamente en GitHub, incluso sin instalar MkDocs. Al crecer el número de funciones se puede añadir generación automática si reduce trabajo real.

## Comparación de herramientas

| Opción | Ventaja | Coste para Malvado | Decisión |
| --- | --- | --- | --- |
| Markdown + MkDocs | Un sitio, enlaces, búsqueda y revisión en PR | Hay que mantener fichas y firmas con disciplina | Base inicial |
| Doxygen | Extrae símbolos y comentarios C; soporta Markdown y salida HTML/XML | Añade generación, posible duplicación y una integración a mantener | Anotar cabeceras con comentarios compatibles; evaluar generación cuando crezca la API C |
| LDoc | Genera documentación Lua y puede procesar módulos Lua escritos en C | Requiere anotar explícitamente funciones registradas en C y mantener otra salida/herramienta | Evaluar más adelante; no necesario para la primera publicación |
| Lua Language Server / LuaCATS | Tipos, firmas y ayuda en el editor a partir de anotaciones | Para un módulo C hacen falta definiciones Lua mantenidas o generadas | Usar definiciones de API Lua como soporte de editor, no como motor obligatorio del sitio |

Fuentes: [Doxygen, comentarios de C](https://www.doxygen.nl/manual/docblocks.html); [LDoc](https://stevedonovan.github.io/ldoc/manual/doc.md.html) y [LuaCATS](https://luals.github.io/wiki/annotations/). LDoc **sí admite** documentar módulos Lua escritos en C, pero su manual indica que no puede deducir automáticamente sus nombres de función; hay que documentarlos. LuaCATS es el formato del servidor Lua Language Server; no afirmar que todas sus versiones sean idénticas a EmmyLua.

## Organización propuesta

```text
README.md                         entrada breve, instalación y enlaces
mkdocs.yml                        configuración futura del sitio
requirements-docs.txt             versión de MkDocs fijada
include/malvado/*.h               declaraciones y comentarios de la API C
bindings/lua/definitions/         definiciones .lua anotadas, solo para tooling
examples/c/                       proyectos y ejemplos C reales
examples/lua/                     proyectos y ejemplos Lua reales
tests/                            pruebas de comportamiento y de ejemplos
docs/site/index.md                inicio público del sitio
docs/site/getting-started/        primeros juegos C y Lua
docs/site/api/c/                  referencia C por módulo
docs/site/api/lua/                referencia Lua por módulo
docs/site/guides/                 tutoriales compartidos y recetas
docs/site/migration/              migración y compatibilidad
```

La ubicación `docs/site/` es una propuesta para **fuentes públicas** del sitio; configurar `docs_dir: docs/site` y un directorio de salida ignorado por Git. Esto evita que MkDocs copie o publique accidentalmente todas las fuentes históricas `docs/api/upstream/` y catálogos grandes. Los planes, fuentes históricas y análisis siguen en sus carpetas actuales de `docs/` y se pueden enlazar desde GitHub; no se mezclan con la referencia de APIs propias. La ruta final del binding debe coincidir con la arquitectura real al implementarse: no crear directorios vacíos solo por esta propuesta.

## Fuente de verdad por lenguaje

**C:** la firma y tipos viven en las cabeceras públicas instalables `include/malvado/`. Documentar junto a cada declaración el propósito, parámetros, unidades, retornos/errores, ownership, ciclo de vida y versión. Usar comentarios `/** ... */` y etiquetas como `@param` y `@return`, compatibles con Doxygen, aunque el sitio inicial se redacte en Markdown. Los nombres y argumentos de la referencia se cotejan con esas cabeceras. No publicar una API C futura como si ya existiera.

**Lua:** la firma pública, tipos y comentarios viven en un archivo de definiciones `.lua` con anotaciones LuaCATS, para un binding implementado en C. Es un archivo de **tipos y tooling**: no debe instalarse ni cargarse como implementación que sustituya a `require("malvado")`. El contrato visible de cada función también se explica en su página Markdown y enlaza a su ejemplo. El registro de funciones del binding, las definiciones y la documentación se actualizan en el mismo cambio; CI comprueba que los nombres exportados coinciden y los tests ejercitan firmas y comportamiento. El inventario de referencia Bennu no forma parte de este contrato.

Las cabeceras C y las definiciones Lua son fuentes de firmas por lenguaje; las páginas Markdown son la fuente narrativa para comportamiento, ejemplos y diferencias entre lenguajes. Los comentarios de cabecera/definición deben mantenerse concisos y enlazar al módulo correspondiente. La comprobación de nombres no demuestra automáticamente que tipos, retornos o semántica sean correctos: revisarlos y probarlos. Evitar un tercer fichero de contratos JSON obligatorio hasta demostrar que genera y verifica ambas salidas con menos mantenimiento. No generar documentación desde una fachada Lua inexistente.

## Ficha de API: contenido mínimo

Para cada función o tipo público y cada variante disponible:

- Nombre, firma exacta del lenguaje, módulo y versión mínima; estado real: implementado o previsto.
- Qué hace, argumentos, tipo, unidades, rangos, valores opcionales, retorno, errores y plataforma.
- En C: ownership, nulabilidad, tiempo de vida de punteros/handles y destructor; en Lua: tipos, tablas/userdata, errores y gestión de referencias.
- Efectos sobre proceso/tick, ejemplos relacionados y enlaces entre la API C y Lua cuando correspondan.
- Enlace a un **ejemplo ejecutable**, con comandos, resultado y recursos necesarios. Un ejemplo puede cubrir varias funciones; no es obligatorio crear un programa distinto por símbolo.

Organizar por módulos —contexto, procesos, señales/mensajes, tiempo, ventana, gráficos, texto, entrada y audio— con índices independientes C y Lua. Mantener juntos los conceptos compartidos en una guía y enlazar sus dos referencias. Los detalles exclusivos de C o de Lua se documentan en su propia referencia.

## Ejemplos como fuente real

En `examples/`, no dentro de bloques mantenidos a mano en cada página. Cada carpeta incluirá README, requisitos, pasos de compilación/ejecución y salida esperada; C incluye `main.c` y configuración de build; Lua incluye `main.lua` y configuración cuando exista CLI. `hello`, procesos/señales/mensajes y un juego pequeño deben tener versiones C y Lua que muestran el mismo comportamiento. Añadir ejemplo mixto para interacción, con límites de tipos de mensajes documentados.

En el sitio inicial, enlazar al archivo real de GitHub y al README del ejemplo. Para la documentación de una release usar enlaces al **tag de esa versión**, no a `main`; para desarrollo se puede enlazar a `main`. Si se incluyen fragmentos directamente más adelante, usar una extracción automatizada de regiones del archivo real, con una dependencia fijada y comprobada: MkDocs básico no incluye por sí solo una directiva genérica para insertar archivos. No duplicar ejemplos de forma que puedan divergir silenciosamente.

Los ejemplos headless deben terminar automáticamente en CI. Los gráficos pueden usar un modo smoke y verificaciones visuales en un runner apropiado. Al menos un ejemplo C se compila contra cabeceras y biblioteca **instaladas** con Lua desactivado; los ejemplos Lua se ejecutan con el runner oficial. El sitio se construye en CI, pero eso no sustituye ejecutar los ejemplos.

## Construcción, CI y publicación

Al implementar la documentación:

```sh
python3 -m pip install -r requirements-docs.txt
python3 -m mkdocs build --strict
python3 -m mkdocs serve
```

Estos comandos son objetivo; aún no existen `mkdocs.yml`, `requirements-docs.txt` ni `docs/site/`. El build genera un directorio de salida fuera de las fuentes y se comprueba en PR. Añadir a CI: dependencias fijadas, build estricto, enlaces internos, detección de firmas/símbolos y ejemplos C/Lua ejecutados. Si se usa un verificador de enlaces externos, ejecutarlo separadamente o controlar fallos temporales de red. No depender solo de nombres: probar resultados/errores/ownership y revisar las páginas junto al cambio de API.

Desplegar Pages únicamente desde el workflow y entorno configurados para publicación; los PR construyen pero no despliegan. Documentación de desarrollo en `/dev/` y documentación estable por minor en `/2.0/` cuando se implemente versionado; ambas deben identificar commit/versión y enlazar a los ejemplos correspondientes. Definir la retención antes de reemplazar versiones. La publicación necesita configuración de Pages y permisos; este plan no modifica configuración externa ni publica nada.

## Hitos y criterios

| Hito | Entrega | Aceptación |
| --- | --- | --- |
| Alpha.2, refactor C | Contratos reales de contexto/procesos C y Lua; ejemplos equivalentes mínimos | Juego C sin Lua, juego Lua y casos mixtos ejecutados; fichas y errores revisados junto a API |
| Alpha.3 | Gráficos, entrada, texto y audio en ambas referencias; sitio MkDocs inicial | Build estricto y enlaces; ejemplos por lenguaje funcionan en targets soportados |
| Alpha.4 | Guías SDK C, runner Lua, CLI y exportación | Crear/compilar/exportar un juego C y uno Lua en rutas verificadas |
| Beta | Completar inventario de símbolos públicos y fichas de ownership | Cada símbolo estable tiene documentación, caso(s) ejecutable(s), prueba de comportamiento y revisión de paridad |
| Release 2.0 | Sitio versionado y fuentes fijadas | Documentación, ejemplos y binarios apuntan a la misma versión; Pages publicado tras configuración |

Doxygen o LDoc se incorporan solo tras una prueba pequeña que confirme cobertura de firmas, ejemplos y mantenimiento; sus salidas no reemplazan automáticamente la documentación narrativa. Si se elige generación, establecer una sola fuente para cada dato y hacer fallar CI cuando la salida o firmas estén desactualizadas.
