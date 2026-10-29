# Versiones propuestas

El cambio de backend, dependencia de Lua, API y distribución justifica una nueva línea **2.0**. El rockspec actual contiene `1.0.0-0`; no se modifica aquí ni se publica una release. Las siguientes etiquetas son propuestas, no versiones entregadas.

| Versión | Resultado visible | Dependencia |
| --- | --- | --- |
| Preparación | Fuentes/licencias revisadas, baseline y decisiones fijadas | Inventario documental actual |
| `2.0.0-alpha.1` | Runtime C + raylib + Lua incluido y hello | Preparación |
| `2.0.0-alpha.2` | Core y API C utilizables sin Lua; procesos, señales, binding Lua y paridad verificados ([en desarrollo](2.0.0-alpha.2.md)) | alpha.1 y [refactor C](../architecture/c-first.md) |
| `2.0.0-alpha.3` | Gráficos, texto, entrada y audio en API C y binding Lua; cuatro ejemplos portados | alpha.2 |
| `2.0.0-alpha.4` | malvado-cli, SDK C y paquetes C/Lua para Linux/macOS; ruta Windows en diseño | alpha.3 |
| `2.0.0-alpha.5` | Perfil Bennu esencial delimitado y documentado | alpha.3 y contratos API |
| `2.0.0-alpha.6` | Prototipo y APK/AAB Android; validación completa de Windows | alpha.4 y alpha.5 |
| `2.0.0-beta.1` | Contrato esencial congelado y documentación verificable | Todas las alphas anteriores |
| `2.0.0` | Estable del perfil esencial y exportadores aceptados | beta y criterios cerrados |
| `2.1+` | Familias extendidas, variantes/export FPG, ZIP montable y mejoras compatibles | Matriz de cobertura por versión |

El perfil esencial se delimita antes de implementar sus familias; no se inventa una promesa de «100% Bennu» para 2.0. Los perfiles extendidos pueden quedar pendientes sin bloquear 2.0 si se documentan. No hay estimaciones de fecha hasta cerrar el primer corte y medir complejidad de formatos y plataformas.

## Secuencia de validación por plataforma

Las alphas 1–5 se pueden cerrar con builds y pruebas obligatorias en macOS y Linux, sin esperar a Android ni Windows. La alpha.6 incluye el prototipo Android de ventana, background/foreground y Lua integrado, así como las pruebas y exportaciones Android. También incorpora Windows a CI y valida el recorrido completo: configuración/compilación, pruebas C/Lua y mixtas, ventana/entrada/audio, CLI, exportaciones y ejecución de paquetes en una máquina Windows limpia. Los fallos de ambas plataformas se corrigen en alpha.6 antes de beta; hasta entonces son objetivos pendientes, no soporte garantizado. GUI se valida en Windows en alpha.6 si se incluye en la distribución de ese momento.

El workflow actual se limita a macOS/Linux; recuperar el job Windows en alpha.6. No confundir un build CI con la aceptación real de instaladores, gráficos o ejecución en máquina limpia.

## Puerta adicional de alpha.2: motor C independiente

El comportamiento Lua existente sigue siendo una base útil, pero **alpha.2 no está cerrada** mientras procesos y señales residan en `runtime.c` y dependan de Lua. Antes de avanzar al cierre: core y API C compilables sin Lua/raylib en modo headless; juego C real y prueba de lifecycle; binding Lua conectado al mismo scheduler; paridad de señales, órdenes, mensajes y cleanup; pruebas C, Lua y mixtas en CI. El backend gráfico raylib se incorpora a la prueba C de ventana. Los detalles y pasos están en [arquitectura C](../architecture/c-first.md).

## Documentación C y Lua

Alpha.2 incluye documentación de contratos y ejemplos mínimos C y Lua; alpha.3 añade la referencia de gráficos y un sitio MkDocs inicial; alpha.4 documenta SDK/CLI; beta completa símbolos y ejemplos y 2.0 publica documentación versionada. La [propuesta de documentación](../development/api-documentation.md) define criterios y herramientas; aún no se ha implementado el sitio.

## CI por versión

Alpha.1 incorpora builds CMake/CTest y comprobación de documentación en GitHub Actions; alpha.2 añade análisis y sanitizers; alpha.3, smoke gráfico y build del sitio; alpha.4, paquetes desktop y candidato de release; alpha.6, Android y la matriz/pruebas completas de Windows. Beta prepara Pages versionado y publicación revisada. Detalles y criterios de aceptación en [CI y releases](../development/ci.md). El workflow de builds, catálogo, sanitizers y prueba gráfica Linux está implementado; falta ejecutarlo en GitHub.

## Línea opcional malvado-gui

| Hito | Resultado | Dependencias |
| --- | --- | --- |
| G0 | Prototipo de toolkit: ImGui/cimgui frente a raygui, DPI, árbol y atlas | Core alpha.3 |
| GUI `0.1.0` / G1 | Proyectos, editor externo, Run/Stop, consola, preview recursos y export desktop | Core/CLI alpha.4 y servicios compartidos |
| GUI `0.2.0` / G2 | Inspector de procesos, Pause y Step tick | G1, hooks del core P2 y protocolo local validado |
| GUI `0.3.0` / G3 | Metadatos, pivotes/puntos/animación, undo y conversión FPG disponible | G2 y pipeline de assets |
| GUI `1.0.0` / G4 | Flujo completo aceptado en hosts soportados | G1–G3; Android requiere CLI alpha.6 |

La versión GUI es independiente de la versión del runtime. Ninguna de estas entregas se considera implementada ni bloquea el lanzamiento del core/CLI. Ver [alcance GUI](../specs/gui.md) y [formatos](../specs/assets.md).

El [estado del primer corte alpha.1](2.0.0-alpha.1.md) describe cómo compilarlo y qué falta.

## Checklist de alpha.1

- [ ] Resolver contradicción de licencias del código existente.
- [ ] Fijar versión exacta/hashes y avisos de Lua y raylib; builds sin descargas implícitas tras bootstrap.
- [ ] Añadir CMake, API C opaca y runner C con Lua embebido.
- [ ] Registrar require("malvado"), process, frame y arranque/cierre mínimos.
- [ ] Mantener corutinas con referencias de registry y cerrar sin fugas.
- [ ] Separar tick y draw; reloj inyectable en pruebas headless.
- [ ] Ejecutar hello en macOS y Linux; smoke Windows se verifica en alpha.6.
- [ ] Mostrar errores de script con traceback y código de salida no cero.
- [ ] Documentar cómo compilar y ejecutar sin LÖVE ni Lua instalado.
- [ ] CI de builds desktop disponibles, CTest con timeout y comprobación de catálogo/documentación.

## LuaRocks

Se conserva el rockspec 1.0 en raíz. Un rock 2.0 es una opción adicional sujeta a implementar y probar un módulo C para Lua externo; la distribución principal será el SDK autónomo. Ver [distribución](../development/distribution.md). CLI y GUI tienen canales propios y no se consideran automáticamente incluidas en ese rock.

## Política de versión

Runtime, binding Lua y CLI publican inicialmente una versión coordinada para reducir combinaciones. La GUI opcional lleva su propia versión y declara qué versiones de proyecto, runtime y protocolo de diagnóstico admite. El formato de proyecto lleva `schema=1`; una versión de documentación «2.0» no cambia ese esquema automáticamente. Mantener `main` como desarrollo, documentación por minor estable y etiquetas Git para la fuente exacta. Changelog solo de cambios entregados; el roadmap permanece separado.
