# CI, documentación y versiones con GitHub Actions

Estado: workflow inicial implementado y ejecutado en GitHub el 2026-10-29. El objetivo es detectar fallos al proponer cambios, generar binarios verificables y publicar documentación y versiones cuando estén listas. No se crea ni publica ninguna release en esta fase.

## Qué se ejecuta y cuándo

| Workflow propuesto | Cuándo | Trabajo | Resultado |
| --- | --- | --- | --- |
| `ci.yml` | PR y push a rama principal; ejecución manual | Compilar, CTest, comprobaciones de código y documentación | Checks visibles; logs y artefactos de diagnóstico ante fallos |
| `docs.yml` | PR y push a rama principal; manual | Regenerar catálogo, comprobar enlaces y construir sitio | PR valida; rama principal puede publicar Pages tras configurar el repositorio |
| `release.yml` | Manual para preparar candidato; tag `v*` para automatizar tras validar el proceso | Compilar, probar, empaquetar, calcular hashes y preparar release | Artefactos por plataforma y draft release para revisión |
| `nightly.yml` | Manual o programado cuando sea útil | Sanitizers, pruebas lentas y más combinaciones | Diagnóstico adicional; no sustituye a CI de PR |

La publicación requiere configurar permisos y Pages en GitHub; el mero hecho de escribir los workflows no configura ni publica el sitio. Los PR externos se ejecutan sin secretos de firma o despliegue. Usar nombres de checks estables para protección de rama cuando el repositorio lo permita.

El archivo `.github/workflows/ci.yml` contiene la matriz macOS/Linux, la comprobación del catálogo, un job de sanitizers y análisis estático, y la prueba gráfica Linux con Xvfb. Los siete jobs pasaron en [GitHub Actions para el commit 09215e8](https://github.com/jepemo/malvado/actions/runs/40908427079). Windows se valida en alpha.6.

## CI inicial: alpha.1

1. **Linux y macOS:** configurar CMake con versiones/toolchains documentados, compilar y ejecutar `ctest --output-on-failure`. Instalar las dependencias de sistema para raylib. Windows se incorpora y valida en alpha.6; sus resultados no bloquean las alphas anteriores.
2. **Raylib/Lua:** CMake descarga Lua 5.4.8 y raylib 5.5 en el directorio de build y verifica sus hashes SHA-256. Cachear la descarga cuando sea posible, sin omitir la verificación. Limitar el análisis de estilo al código del proyecto, excluyendo `vendor/` y `_deps/`.
3. **Estática y estilo:** habilitar advertencias del compilador para `src/`, incorporar `clang-format --dry-run --Werror` cuando exista configuración aplicada al código y después `clang-tidy` sobre `compile_commands.json` con un conjunto acordado de reglas. Añadir `luacheck` para ejemplos y pruebas Lua cuando exista configuración. Las advertencias y excepciones conocidas se documentan; no presentar una herramienta aún no configurada como check ya operativo.
4. **Pruebas de memoria:** ASan/UBSan en una configuración de Linux compatible al ampliar tests de procesos; separar restricciones de plataforma. Añadir escenarios de creación/baja, errores y cleanup de corutinas.
5. **Documentación:** descargar las fuentes fijadas con `python3 docs/tools/fetch_api_sources.py`, ejecutar `python3 docs/tools/catalog_api.py`, comparar los archivos generados con el repositorio y fallar si difieren; ambos comprueban los hashes de las fuentes. Validar enlaces internos y construcción del sitio cuando exista el generador. Comprobar los archivos generados también si todavía están sin seguimiento en la fase inicial.
6. **Ejemplo gráfico:** el test headless no demuestra que se abra una ventana. Añadir smoke gráfico en runners adecuados, por ejemplo Linux con servidor virtual, y pruebas en sistemas reales. La aceptación de macOS y audio/entrada se registra aparte cuando no sea fiable automatizarla; Android y Windows se validan en alpha.6.

Matriz inicial recomendada: Linux x86_64 y macOS arm64; incorporar Windows x86_64 en alpha.6, con nombres explícitos de runner/arquitectura y versiones fijadas al implementar. Confirmar disponibilidad de runners para este repositorio; si una arquitectura no está disponible, dejar su check pendiente o usar un runner apropiado, sin etiquetar el destino como validado. No lanzar el hello interactivo indefinidamente en CI: añadir un modo smoke con salida automática.

## Calidad de los checks

- Cada job falla ante una compilación, test o regeneración incorrecta; no ocultar fallos con `continue-on-error` en checks requeridos. Añadir límites de tiempo a tests y jobs para detectar procesos Lua que no ceden.
- Compilar y probar en build limpio; directorios de build y cachés no se suben al repositorio. Guardar logs y resultados de tests en artefactos temporales de CI con retención acotada.
- Cachear por OS, arquitectura, compilador, versiones y hash de dependencias. Una caché no es prueba de reproducibilidad y no reemplaza compilaciones limpias periódicas.
- Formato y análisis para código propio; los archivos externos fijados conservan su formato y licencia. Versiones de herramientas y reglas se documentan y actualizan mediante PR.
- Añadir checks de ejemplos, activos y GUI solo cuando se implementen; por ejemplo, generar un proyecto CLI y ejecutar su test de humo cuando exista la plantilla.

## Documentación de ambas APIs

[Plan de documentación](api-documentation.md): exigir en cada PR que cambie una API pública que actualice sus firmas y fichas C/Lua correspondientes, y ejecutar ejemplos relacionados. El job de documentación construirá MkDocs en modo estricto, comprobará enlaces y nombres entre binding y definiciones Lua. La extracción automatizada de ejemplos y verificadores de firmas se implementarán y fijarán antes de afirmarlos como checks existentes; el build del sitio no sustituye las pruebas de ejemplos.

## Documentación y GitHub Pages

Markdown en `docs/` sigue siendo la fuente. Como se propone en [publicación](publishing.md), fijar MkDocs y usar inicialmente su tema por defecto al configurar el sitio. En PR se regenera el catálogo y se construye la documentación sin desplegar. En push a la rama principal, una vez configurado Pages, desplegar la documentación de desarrollo. Mantener documentación de versiones estables desde tags en `/2.0/` y desarrollo en `/dev/` cuando exista un sistema de versiones; definir la política de retención antes de borrar versiones publicadas.

[GitHub documenta](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages) la construcción y despliegue con Actions. Un job de Pages necesita `pages: write` e `id-token: write`; concederlos solo al job de despliegue, no a PR. La generación de referencia Lua/C se incorpora cuando existan contrato propio y herramientas configuradas, sin convertir automáticamente los símbolos Bennu en API de Malvado.

## Generación de versiones

La release debe representar una revisión exacta y superar CI. Al principio generar artefactos candidatos desde un workflow manual, revisar su instalación y luego crear un **draft release**; cuando el proceso sea fiable, permitir que un tag `v2.0.0-alpha.1` dispare el mismo pipeline. Alpha/beta son prereleases. La publicación del draft, firma de producción y cualquier subida a tiendas quedan sujetas al proceso de release acordado.

| Entrega | Primer artefacto previsto | Comprobación antes de etiquetar como soportado |
| --- | --- | --- |
| Runtime desktop | Ejecutable/paquete para Linux x86_64, Windows x86_64 y macOS arm64 | CTest, arranque de ejemplo, dependencias incluidas y prueba en máquina limpia |
| SDK/CLI | Herramienta y runtimes para los destinos disponibles | Proyecto nuevo → run → export → ejecutar paquete |
| Android | APK debug para pruebas; APK firmado/AAB para release cuando exista exportador | Instalar y probar ciclo de vida/touch; validar AAB; configuración de firma aparte |
| GUI | Paquetes de escritorio con su propia versión | Recorrido de proyecto, preview, ejecución y export; compatibilidad con runtime declarada |

No fabricar binarios de destinos que todavía no están implementados. No confundir un binario de runner alpha con un juego exportado por `malvado-cli`. El pipeline guarda para cada candidato: tag/commit, versión, plataforma/arquitectura, toolchain, dependencias, licencias, checksum SHA-256 y notas de cambios. `CHANGELOG` describe únicamente lo entregado. Verificar que la versión del tag corresponde a los metadatos antes de empaquetar; no sobrescribir artefactos de una release existente.

macOS necesita firma y notarización para distribución según el canal elegido; Android necesita claves para sus variantes firmadas. Mantener credenciales en secretos/entornos protegidos solo para jobs de release autorizados; CI ordinario no requiere claves. Primero distribuir artefactos de prueba sin declarar una experiencia de instalación validada. No prometer binarios idénticos entre builds firmadas: registrar insumos y comparar resultados reproducibles cuando sea viable.

## Verificación del core C y binding Lua

Añadir a alpha.2 un job limpio con Lua y raylib desactivados: compilar e instalar la biblioteca C con backend headless, compilar un juego C consumidor de las cabeceras instaladas y ejecutarlo. Confirmar que no se descargan ni enlazan Lua/raylib. En paralelo, compilar runner/binding oficial, ejecutar las pruebas Lua existentes y pruebas de paridad C/Lua y mixtas; correr sanitizers. El smoke gráfico C/Lua con raylib se valida por plataforma. No marcar estos checks como existentes hasta implementarlos. Detalles en [arquitectura C](../architecture/c-first.md).

## Distribución y LuaRocks

El [plan de distribución](distribution.md) define SDK, CLI, GUI y rock opcional. Un posible rock 2.0 requiere un módulo C cargable por Lua externo, instalación CMake y pruebas de ABI: el runner actual con Lua embebido no es ese módulo. No añadir un job de publicación de rock hasta tenerlo; después ejecutar lint, instalación y prueba en árboles limpios y revisar licencias antes de cualquier subida.

## Permisos y dependencias de Actions

En CI de PR usar `contents: read` o menos, sin secretos; no ejecutar código de PR no confiable con permisos de escritura. Jobs de publicación necesitan permisos mínimos y separados (`contents: write` para draft release; permisos de Pages solo para su deploy). Fijar las acciones externas a SHA de commit completo, procedente del repositorio original, y revisarlas periódicamente, como recomienda [GitHub](https://docs.github.com/en/actions/reference/security/secure-use). No poner credenciales en logs, artefactos o ejemplos.

## Orden de adopción

| Momento | Implementar | Puerta de aceptación |
| --- | --- | --- |
| Alpha.1 actual | `ci.yml`: builds desktop posibles, CTest headless, timeout, catálogo y avisos de compilador | PR de prueba muestra checks verificables en cada runner disponible; documentar faltantes |
| Alpha.2 | Formato/análisis acordados; sanitizers y tests ampliados del scheduler | CI falla ante errores reales y publica logs útiles |
| Alpha.3 | Smoke gráfico acotado y ejemplos de recursos; build del sitio | Ejemplos CI terminan automáticamente; documentación sin enlaces internos rotos |
| Alpha.4 | Paquetes macOS/Linux y draft de candidato; pruebas de CLI de extremo a extremo | Artefactos instalados y ejecutados en hosts limpios; checksums correctos |
| Alpha.6 | Build Android y validaciones APK/AAB; añadir Windows a CI y validar runtime, C/Lua, GUI si se distribuye, CLI y paquetes | Android: emulador/dispositivo y lifecycle; Windows: checks CI, prueba gráfica y ejecución en Windows limpio; firma release aparte |
| Beta/2.0 | Pages versionado, automatización de tags y criterios de publicación | Candidato completo revisado, documentación y artefactos corresponden al mismo commit |
| GUI | Jobs de build y smoke desktop al iniciar G0/G1 | CI cubre toolkit elegido y separa GUI de runtime |

El CI ayuda a decidir si una versión está lista; no sustituye la revisión de licencias ni las pruebas de distribución y experiencia gráfica en máquinas reales.
