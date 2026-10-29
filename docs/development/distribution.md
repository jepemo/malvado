# Distribución: runtime, herramientas y LuaRocks

Estado: propuesta; no hay paquete 2.0 ni herramientas publicadas. Este documento distingue los productos distribuidos y las tareas para ponerlos a prueba.

## Decisión propuesta

El canal principal de Malvado 2 será un **SDK descargable por plataforma**, con biblioteca y cabeceras C, binding/runner Lua y, cuando existan, `malvado-cli` y plantillas; la GUI tendrá descarga de escritorio separada o un paquete SDK opcional. Los juegos Lua exportados llevan el runtime y Lua incorporados; los juegos C incluyen su runtime nativo sin exigir Lua. Así, jugar no requiere instalar Lua ni LuaRocks.

Se propone evaluar una distribución **opcional** en LuaRocks para desarrolladores que ya usan Lua: depende de implementar y probar un módulo C cargable en un intérprete externo. El runner actual `malvado-run` contiene su propia VM y registra `malvado` en ella; eso no lo convierte automáticamente en un módulo que pueda cargarse desde cualquier `lua`. No anunciar `luarocks install malvado` para 2.0 hasta que ese módulo y su rockspec funcionen.

## Qué ocurre con el rockspec 1.0

`malvado-1.0.0-0.rockspec` en la raíz es una convención válida y útil: `luarocks make` puede buscar un rockspec en el directorio actual. Su [documentación](https://github.com/luarocks/luarocks/blob/main/docs/creating_a_rock.md) también permite pasar un archivo concreto y explica que el nombre debe coincidir con paquete y versión. No es obligatorio almacenar todos los rockspecs en la raíz del repositorio.

El archivo actual apunta a `v1.0`, usa módulos Lua de `malvado/`, depende de Lua >= 5.1 y describe la versión LÖVE. Su presencia no prueba que la release exista en LuaRocks ni que su fuente/tag sean accesibles hoy. La revisión y la discrepancia de licencias del código histórico se documentan en el [roadmap](../planning/roadmap.md).

**Por ahora se conserva donde está**: sigue siendo el único rockspec y no hay ningún problema que justifique romper el flujo existente. Cuando aparezcan varios rockspecs reales, considerar `packaging/luarocks/` para agruparlos y usar el nombre explícito, por ejemplo `luarocks make packaging/luarocks/malvado-1.0.0-0.rockspec`. Verificar en CI, desde la raíz y con el checkout histórico adecuado, que rutas de código y `source.dir` sigan funcionando; no moverlo solo por estética ni editar un rockspec ya publicado suponiendo que cambia lo que está en el servidor.

## Productos y canales

| Producto | Canal principal | LuaRocks | Destinatario |
| --- | --- | --- | --- |
| Core y API C | SDK con cabeceras y biblioteca nativa por plataforma | No requiere LuaRocks ni Lua | Juegos y herramientas C |
| `malvado-run` / runtime | SDK o binario desktop con Lua incorporado | Opcional; definir un paquete específico solo si se demuestra útil | Quien ejecuta y desarrolla juegos |
| Binding Lua `malvado` | Incluido en runner | Rock opcional con módulo C para Lua externo, pendiente de diseño | Proyectos que ya gestionan Lua/LuaRocks |
| `malvado-cli` | SDK/archivo de release por plataforma | Posible rock independiente posterior, solo si se valida instalación de ejecutable y dependencias | Creador de proyectos y exportaciones |
| `malvado-gui` | Aplicación desktop independiente o distribución SDK opcional | No prioritario: toolkit y empaquetado nativo complican esta vía | Creador que prefiere interfaz gráfica |
| Juego exportado | ZIP/carpeta desktop, APK/AAB Android | No necesario para el jugador | Jugador |

Nombres de rocks separados, por ejemplo `malvado` y `malvado-cli`, son propuestas y requieren comprobar disponibilidad y política de compatibilidad antes de publicar. El rock de una biblioteca Lua no instala necesariamente el SDK completo, exportadores ni GUI. Un SDK offline o descargable por destino deberá definir sus binarios y dependencias en el manifiesto.

## Decisión técnica pendiente para el rock 2.0

Un módulo externo deberá enlazarse contra la ABI del intérprete que lo cargue y ser probado con la versión Lua soportada. La propuesta inicial es Lua 5.4; no asumir compatibilidad 5.1/LuaJIT ni cargar una segunda copia independiente de Lua en el mismo módulo. El runner autónomo enlaza el Lua descargado y compilado para la distribución; un juego C enlaza el core sin Lua. Compartir obligatoriamente el core C entre el juego nativo y el binding, separando el host/registro de módulos del ejecutable; evitar dos implementaciones distintas de las reglas del motor.

El [formato de rockspec](https://github.com/luarocks/luarocks/blob/main/docs/rockspec_format.md) admite backends `builtin`, `cmake` y otros, módulos C, dependencias externas e instalación de archivos. Elegir el backend después de añadir targets de módulo y reglas de instalación a CMake; el CMake actual solo genera el ejecutable, así que añadir un rockspec 2.0 ahora sería engañoso. Decidir si raylib se construye con el módulo o se declara dependencia externa, fijando versiones y requisitos de plataforma.

El binding externo necesita especificar quién posee la ventana y el bucle, cómo se inicializa y cierra, y qué ocurre al usarlo en otro host gráfico. No asumir que funcionará embebido en LÖVE o mezclado con otro loop. El mismo contrato Lua debe dejar claras las diferencias de host. Linux, macOS y Windows necesitan pruebas de carga del módulo y de ejecución real, además de `luarocks lint`, instalación en un árbol limpio y desinstalación.

Para evitar una actualización inesperada de juegos existentes, las versiones 1.x y 2.x deben declarar explícitamente sus rangos y guía de migración. Si comparten el paquete `malvado`, 2.0 debe identificar la ruptura de API y motor; decidir antes de publicar cómo se selecciona 1.x y si conviene un nombre alternativo. Verificar el comportamiento real del resolver de LuaRocks y el estado de los nombres, sin presuponerlo.

## Versiones y CI

Runtime y CLI mantienen la coordinación propuesta de versiones; GUI tiene versión propia y matriz de compatibilidad. Los artefactos de release registran SO, arquitectura, versiones de runtime/Lua/raylib, hashes y avisos. Cuando el rock exista, generar rockspec desde fuente fijada por tag, probar `luarocks lint` y `luarocks make` en árboles temporales de cada plataforma soportada. Probar también el artefacto fuente (`luarocks pack`); si se publican rocks binarios, comprobarlos por plataforma y ABI Lua. Publicar en LuaRocks solo después de esas pruebas y de resolver licencias, como parte del proceso de release. La subida requiere cuenta/credenciales y es independiente de generar candidatos en CI.

No hay rockspec 2.0, targets de instalación LuaRocks, CLI ni GUI implementados actualmente. El [plan de CI](ci.md) y el [roadmap](../versions/README.md) recogen estas dependencias.
