# Plan de migración de Malvado 2

Estado: propuesta técnica; ninguna fase de implementación se da por terminada. Actualizado: 2026-10-29.

## Objetivo

Sustituir LÖVE por raylib y trasladar a C el runtime, scheduler, datos de procesos, recursos, render, entrada, sonido, colisiones y funciones de la API. Crear una biblioteca de motor y API pública en C, usable por juegos C sin Lua. Incluir en el proyecto el binding Lua oficial y el runner con Lua embebido: los juegos Lua no requieren instalar Lua, LÖVE o LuaRocks. Incorporar `malvado-cli` para crear proyectos y exportar Linux, Windows, macOS y Android. Añadir `malvado-gui` como taller opcional de escritorio para proyectos, recursos e inspección de procesos, apoyado en los mismos servicios C.

Esta propuesta sustituye el antiguo plan que exigía «No C changes», bindings raylib de terceros y LuaRocks como vía principal. Se conserva en [history/PLAN-love-to-raylib-lua.md](../history/PLAN-love-to-raylib-lua.md).

## Punto de partida comprobado

- `malvado/core.lua` crea una coroutine por instancia, implementa `frame()` con `coroutine.yield()` y reanuda desde `love.draw`.
- Los módulos llaman directamente a `love.graphics`, `love.window`, `love.timer`, `love.keyboard`, `love.mouse` y `love.filesystem`.
- Se mezclan simulación y dibujo; la misma colección de procesos se reordena por `z`. La nueva arquitectura separará orden de ejecución y orden de dibujo.
- `collision()` comprueba si el origen de otro proceso cae dentro de una caja; no es todavía una colisión completa entre sprites.
- Hay hello world, menú demo, asteroides y benchmark. Asteroides y benchmark también llaman a `love.*` directamente: migrar solo la biblioteca no basta.
- `load_fpg()` contiene trabajo experimental de ZIP; `fpg()` acepta directorios/spritesheets. No asumir soporte binario FPG clásico.
- El rockspec declara `1.0.0-0` y Lua >= 5.1. Esto no prueba que exista una distribución 1.0 completa.
- Hay discrepancias de licencia: LICENSE/rockspec indican Apache 2.0, mientras varios encabezados Lua indican GPLv3+. Resolver la procedencia antes de publicar; conservar los avisos mientras tanto.

## Decisiones propuestas

| Asunto | Propuesta | Razón |
| --- | --- | --- |
| Implementación | C11 + CMake; raylib detrás de adaptador C | Core nativo y builds de escritorio/Android |
| Lenguaje de juegos | API C completa y binding Lua oficial, ambos de primera clase | Permitir juegos C sin VM y juegos Lua con runtime autónomo |
| Versión Lua inicial | Evaluar Lua 5.4 como base conservadora; confirmar parche y soporte al fijar dependencias | API C de corutinas y limpieza disponibles; no prometer LuaJIT/5.1 |
| Procesos | Scheduler C único; callbacks con estado para C y corutinas gestionadas por el binding Lua | Los dos lenguajes comparten ciclo de vida, señales y prioridades |
| Referencia | BennuGD2 principal, BennuGD clásico histórico y DIV/Fénix como contraste | Inventarios verificables sin mezclar firmas de distintas generaciones |
| Compatibilidad | Perfil Malvado antiguo y perfiles de familia DIV explícitos | Grados, escala, textos y temporización ya difieren |
| Distribución | Ejecutable con Lua/raylib; módulo C de Lua opcional posterior | Evita dependencia del entorno del jugador |
| Herramientas | `malvado-cli` en C, biblioteca de empaquetado compartida | Cumple prioridad C; lógica de proyectos reutilizable |
| GUI | Taller de proyectos/recursos/procesos; evaluar ImGui+cimgui+adaptador raylib | C en servicios y core; C++ acotado a dependencias UI; raygui como alternativa C estricta |
| Assets | PNG+JSON editable, FPG como compatibilidad y ZIP como contenedor | Preservar IDs/puntos sin crear otro binario gráfico propietario |
| CI y releases | GitHub Actions para builds, pruebas, análisis, docs y artefactos por versión | Detectar regresiones por PR y generar candidatos verificables |
| Documentación | APIs C y Lua con ejemplos ejecutables, Markdown y MkDocs en Pages | Dos referencias de primera clase, revisión en PR y búsqueda versionada |

No reimplementar la VM de Lua. Lua ya está implementado en C. Tampoco usar threads del sistema para simular procesos DIV. La pila de un script Lua se conserva mediante la VM, aunque el scheduler sea C. El core C deberá ejecutarse sin Lua con procesos nativos de estado explícito; no se promete suspender arbitrariamente una pila C. Esto es un requisito aún pendiente, detallado en [arquitectura C y binding Lua](../architecture/c-first.md).

## Fases y puertas de salida

| Fase / versión propuesta | Trabajo | Evidencia necesaria para cerrar |
| --- | --- | --- |
| P0 / preparación | Resolver licencias; fijar fuentes, hashes y versiones Lua/raylib; medir ejemplos actuales; elegir comportamiento legacy | Matriz de API clasificada, capturas y trazas baseline, build de dependencias reproducible |
| P1 / 2.0.0-alpha.1 | CMake, Lua incluido, contexto C, ventana raylib, runner, `process/frame`, hello y CI inicial de builds y CTest | Hello abre/cierra sin LÖVE ni Lua instalado en macOS y Linux; scheduler funciona headless |
| P2 / 2.0.0-alpha.2 | Extraer scheduler, procesos, señales, mensajes y API pública C; conectar binding oficial Lua; inspección y documentación de ambas APIs | Juego C sin Lua, paridad C/Lua, ejemplos y contratos, trazas deterministas, referencias liberadas y pruebas de memoria |
| P3 / 2.0.0-alpha.3 | Render/recursos PNG+JSON, entrada, texto y audio en API C y binding Lua; shim legacy; migrar los 4 ejemplos; preparar GUI G0 | Demos C/Lua equivalentes, sin `love.*`; imágenes comparables; benchmark documentado |
| P4 / 2.0.0-alpha.4 | CLI, SDK C, runner Lua y paquetes de escritorio; habilita GUI G1 | Crear → jugar → exportar → ejecutar proyectos C y Lua en macOS y Linux; Windows pendiente de alpha.6 |
| P5 / 2.0.0-alpha.5 | Compatibilidad esencial Bennu, scroll, regiones, primer importador FPG de variante fijada, adaptador Aseprite básico, colisiones y extras priorizados | Cada símbolo del perfil esencial tiene contrato, ejemplo y comprobación de semántica |
| P6 / 2.0.0-alpha.6 | Export Android y validación completa Windows: runtime, API C/Lua, CLI y empaquetado | Android: APK y AAB, touch y lifecycle; Windows: CI, ejemplos C/Lua, CLI y paquetes en máquina limpia; resolver fallos antes de beta |
| P7 / 2.0.0-beta.1 | Congelar API esencial, docs publicables, empaquetado y errores consistentes | Matriz de soporte sin ambigüedades, ejemplos completos y pruebas en OS soportados |
| P8 / 2.0.0 | Estabilización y guía de migración | Criterios de beta cerrados, releases reproducibles y aceptación de juegos muestra |
| 2.1+ | Ampliar familias y funciones históricas pendientes | Versiones por perfil; nunca declarar compatibilidad total sin completar su matriz |

Las alphas 1–5 se cierran con verificaciones de macOS y Linux; el prototipo, las pruebas y correcciones de Android y Windows se concentran en alpha.6 antes de beta. Ambas plataformas siguen siendo objetivos del producto, pero no se anuncian como soportadas hasta validarlas. La primera versión estable no queda bloqueada por CD-ROM, punteros crudos, publicidad o funciones históricas poco útiles; deben aparecer como pendientes o no soportadas, con motivo. El objetivo de investigar toda la API se mantiene separado de implementar todo en una sola entrega.

## Documentación de APIs

C y Lua tendrán guías de inicio, referencia por módulo y ejemplos ejecutables equivalentes. Se propone Markdown en Git + MkDocs + GitHub Pages, con firmas C en cabeceras públicas y definiciones LuaCATS para el binding. Véase [plan de documentación](../development/api-documentation.md) para fuentes, CI y versionado. La documentación propuesta no está implementada todavía.

## CI y publicación de versiones

[Plan de GitHub Actions](../development/ci.md): empezar en alpha.1 con builds y pruebas headless por plataforma disponible y verificación del catálogo; añadir análisis estático, sanitizers y smoke gráfico en fases posteriores. Preparar binarios de candidato y draft releases al existir empaquetado; documentar y publicar Pages mediante un job separado. Los checks actuales y las plataformas aún pendientes se indican explícitamente.

## Línea de herramientas gráficas

Ver [malvado-gui](../specs/gui.md): G0 valida toolkit/DPI/tablas/árboles; G1 crea proyectos, abre editor externo, ejecuta y exporta; G2 inspecciona procesos y controla ticks; G3 edita metadatos/animaciones y reutiliza importadores FPG. GUI opcional, con versión propia y sin bloquear el runtime 2.0. Primera entrega con juego en proceso y ventana separados, sin IDE completo ni preview incrustada obligatoria.

Los formatos y la distribución se concretan en [assets](../specs/assets.md): PNG+JSON durante desarrollo, FPG para interoperabilidad con alcance por variante y ZIP para transporte/distribución. Export FPG y ZIP montable por runtime son capacidades posteriores distintas del primer lector FPG.

## Organización prevista del código

```text
include/malvado/       API C y tipos opacos
src/core/             procesos, scheduler, mensajes, reloj, recursos
src/modules/          funciones de gráficos, texto, entrada, sonido, utilidades
src/platform/raylib/   integración de ventana, GPU, audio y plataforma
src/bindings/lua/     registro C de API y userdata
src/tooling/          servicios C compartidos de proyectos, assets y exportación
src/cli/              interfaz de comandos a esos servicios
src/gui/              UI, preferencias y adaptador gráfico aislado
src/debug/            snapshots/comandos y protocolo local opt-in
lua/malvado/          fachada mínima y compatibilidad opcional
build/_deps/          dependencias descargadas por CMake, ignoradas por Git
cmake/                dependencias/toolchains
platform/android/     plantilla Activity/Gradle/NDK
examples/             juegos ejecutables y assets
api/                  contrato propio de Malvado cuando se formalice
```

Esta estructura es propuesta; todavía no se crean directorios de código vacíos. La investigación queda en `docs/api`, con los originales externos aislados de la compilación.

## Riesgos y mitigación

- **Semántica de procesos:** especificarla antes de portar, con trazas de orden de ejecución y tiempos. No trasladar incidentalmente errores del core viejo al perfil nuevo.
- **Datos entre Lua y C:** userdata con generación, ownership y referencias de registry; nunca punteros de memoria como identificadores públicos.
- **Raylib no cubre toda la familia DIV:** FPG/MAP/FNT/PAL, colisión por máscara, scroll y Mode 7 necesitan lógica propia.
- **Documentación histórica incompleta:** exportaciones prueban nombres y tipos, no nombres de argumentos ni comportamiento. Leer implementación y ejemplos antes de completar cada ficha.
- **Alcance GUI:** validar primero el taller enfocado; no convertir la migración en un IDE completo. La dependencia C++ de ImGui queda fuera del runtime/CLI y juegos.
- **Fidelidad de recursos:** conservar IDs, puntos, índices/paletas y offsets; PNG RGBA por sí solo no garantiza semántica histórica.
- **Multiplataforma:** toolchains por destino, smoke tests nativos y SDK Android fijado; no prometer compilación cruzada universal desde un único equipo.

## Siguiente implementación recomendada

Continuar alpha.2 con el [refactor C](../architecture/c-first.md): contexto y juego C sin Lua, scheduler nativo y binding Lua conectado al mismo scheduler. Conservar los tests y comportamientos Lua ya existentes y resolver también los requisitos abiertos de alpha.1 antes de cerrar. Ver [versiones](../versions/README.md) para checklist y [CLI](../specs/cli.md) para exportadores.
