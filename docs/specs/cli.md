# malvado-cli: proyectos y exportadores

Estado: propuesta incorporada a Malvado 2. Destinos solicitados: **Linux, Windows, macOS y Android**. La CLI y estos comandos aún no existen.

## Experiencia objetivo

```sh
malvado-cli new mi-juego --template starter
cd mi-juego
malvado-cli doctor
malvado-cli run
malvado-cli check
malvado-cli export --target linux-x86_64 --output dist/linux
malvado-cli export --target windows-x86_64 --output dist/windows
malvado-cli export --target macos-arm64 --output dist/macos
malvado-cli export --target android-arm64 --format apk --output dist/android
malvado-cli export --target android-arm64 --format aab --release --output dist/android
```

`malvado-cli --version` muestra versión CLI, runtime, formato de proyecto y dependencias fijadas. `export` usa un runtime compatible para el destino: genera un paquete del juego con ejecutable nativo y Lua incluido. No convierte automáticamente el código Lua en código máquina. Los scripts serán fuente Lua empaquetada inicialmente; bytecode opcional requeriría controlar versión de VM, arquitectura y compatibilidad, y no se presentará como protección del código.

El jugador recibe el paquete; no necesita CLI, Lua, LuaRocks o LÖVE. El autor podrá exportar con runtimes precompilados del SDK o construirlos con toolchains documentados.

## Comandos propuestos

| Comando | Función | Hito |
| --- | --- | --- |
| `new NAME --template NAME` | Genera juego, configuración, README y assets mínimos | alpha.4 |
| `run [PATH]` | Valida y ejecuta en runtime local compatible | alpha.4 |
| `check [PATH]` | Sintaxis Lua, manifiesto, assets, identificadores y dependencias | alpha.4 |
| `doctor [--target TARGET]` | Detecta SDK, versiones, herramientas y falta de runtimes | alpha.4; Android alpha.6 |
| `export --target TARGET` | Empaqueta para destino con manifiesto y checksums | alpha.4/alpha.6 |
| `templates list` | Describe plantillas disponibles sin red | alpha.4 |
| `sdk list` | Lista runtimes/toolchains instalados y compatibilidad | alpha.4 |
| `sdk install VERSION --target TARGET` | Descarga explícita y comprobada de runtimes publicados | Posterior, cuando haya releases verificadas |
| `build --target TARGET` | Construye runtime nativo desde fuentes fijadas | Posterior al empaquetado con SDK |
| `assets import` | Convierte el subconjunto FPG/Aseprite validado a la biblioteca PNG+JSON | alpha.5 |
| `assets export --format fpg` | Exporta una variante FPG elegida y documenta pérdidas | 2.1+, opcional |

`check` no demuestra que un juego funcione ni debe ejecutar código arbitrario para analizarlo. `run` sí ejecuta el proyecto. Códigos de salida: 0 éxito, 1 fallo de operación/juego, 2 uso o configuración inválidos; añadir JSON estructurado para CI cuando se cierre el contrato de errores.

## Proyecto generado

```text
mi-juego/
  malvado.toml
  main.lua
  assets/
  README.md
  .gitignore
```

Ejemplo de configuración propuesta (no hay parser aún):

```toml
schema = 1

[project]
name = "Mi juego"
id = "org.example.mijuego"
version = "0.1.0"
entry = "main.lua"

[runtime]
version = "2.0.0-alpha.4"
api_profile = "malvado-2"

[window]
width = 800
height = 600
title = "Mi juego"

[assets]
include = ["assets/**", "*.lua"]

[android]
application_id = "org.example.mijuego"
version_code = 1
orientation = "landscape"
```

La configuración de ventana se aplica como default a `mv.start`; sus opciones explícitas prevalecen. Validar entry, rutas relativas, IDs por plataforma, versión y perfiles. Añadir archivo lock con revisión/hash del SDK y herramientas cuando exista resolución de dependencias. El esquema de proyecto evoluciona independientemente de la versión del runtime.

Plantillas propuestas: `starter`, `processes`, `asteroids`; `android-touch` tras validar móvil. La plantilla starter no necesita descargas ni assets externos. Las demás solo incorporan assets con procedencia y licencia registradas. Las plantillas se prueban como juegos reales, evitando copiar ejemplos que luego diverjan.

## Uso compartido por malvado-gui

La [GUI](gui.md) consume los mismos servicios C de proyecto, configuración, plantillas, jobs, validación y exportación. Si usa inicialmente un subproceso CLI, necesita salida estructurada y protocolo de trabajos, no parsear mensajes traducidos. El runner sigue siendo un proceso separado; el diagnóstico del juego usa snapshots y comandos entre ticks. La CLI funciona sin toolkit gráfico y los exports no incluyen ImGui.

## Recursos y empaquetado

Ver [decisión de formatos](assets.md): PNG+JSON como fuente, adaptadores FPG/Aseprite y ZIP como contenedor. Ejemplos de comandos propuestos, aún no disponibles:

```sh
malvado-cli assets import --format fpg --input origen.fpg --output assets/importado
malvado-cli assets import --format aseprite-json --input sheet.json --output assets/animacion
malvado-cli assets export --format fpg --variant VARIANTE --input assets/importado --output dist/graficos.fpg
```

La variante concreta y sus límites se documentarán al implementarla. `assets import` no renumera gráficos y emite informe de conversión/pérdidas; `assets export` FPG es posterior y puede rechazar datos no representables. ZIP de entrega no significa ZIP montable por el runtime; Android usa assets del APK sin contenedor ZIP extra por defecto.

## Implementación en C

`src/cli` consume bibliotecas C de configuración, validación, empaquetado y lanzamiento. Incorporar una plantilla C además de la Lua y un proyecto con lenguaje explícito: C requiere SDK, cabeceras, biblioteca y toolchain del destino; Lua usa el runner con VM incorporada. `run` y `export` deberán comprobar cada ruta en CI; su implementación pertenece a alpha.4 y Android a alpha.6. Ver [arquitectura C](../architecture/c-first.md). Plantillas incluidas en el binario o SDK, con variables limitadas; no un motor de plantillas que ejecute código del proyecto. TOML requiere elegir un parser C pequeño con licencia compatible y fijarlo; no implementar un parser completo accidentalmente.

Lanzar herramientas con arrays de argumentos, sin concatenar comandos shell a partir de nombres/rutas. Soportar espacios y Unicode. `new` falla si el destino ya contiene archivos; `export` usa staging y publicación atómica del resultado, sin borrar el proyecto. Repetir una exportación solo reemplaza los artefactos identificados como propios. Directorios de build, credenciales y `.git` no entran en el paquete. Manifest de archivos, hashes, licencias y versiones junto al artefacto.

## Contrato por destino

| Destino | Primer artefacto | Herramientas/particularidades | Verificación |
| --- | --- | --- | --- |
| Linux x86_64 | Carpeta/archivo comprimido con ejecutable y assets | Baseline glibc explícita; librerías necesarias/rpath; arm64 después | Equipo limpio sin Lua; input, audio y cierre |
| Windows x86_64 | ZIP portable con `.exe`, assets y DLL necesarias | Build nativo CI o MinGW validado; dependencias de runtime incluidas | Windows limpio sin herramientas de desarrollo |
| macOS arm64 | `.app` en ZIP; x86_64 o universal después | Info.plist, icono y bundle de recursos; firma/notarización de release en macOS | Lanzamiento desde Finder y bundle autocontenido |
| Android arm64 | APK debug; APK release/AAB | SDK/NDK/JDK/Gradle fijados, ABI, Activity, assets y biblioteca nativa | Instalar/ejecutar en dispositivo o emulador, validar bundle |

No todos los destinos tienen todas las arquitecturas en 2.0. La CLI debe rechazar combinaciones no soportadas con un diagnóstico útil. AAB es un formato de distribución que requiere verificación específica y no se instala directamente como APK.

La verificación y soporte efectivo de Windows se reservan a alpha.6; en alpha.4 se aceptan macOS y Linux. Escritorio no implica cross-compilar desde cualquier SO. Primer enfoque: builds nativos en CI por destino y SDKs versionados; el exportador selecciona el runtime. Android necesita su toolchain aunque el host sea macOS/Linux/Windows. La matriz de hosts soportados se publica tras probarlos.

## Android: trabajo de motor además del ZIP

La documentación de [build Android de raylib](https://github.com/raysan5/raylib/wiki/Working-for-Android) será la referencia del prototipo; fijar la plantilla y toolchain en alpha.6, sin trasladar comandos de una versión no probada al producto.

- Host nativo integrado con la Activity de la plantilla seleccionada; empaquetar Lua y todos los módulos C como bibliotecas del juego, sin intérprete instalado.
- Entrada touch con coordenadas de pantalla lógica, DPI, orientación, botón Back y teclado de pantalla cuando proceda. Asteroides con teclado no prueba Android.
- Entrada de background/foreground: pausar simulación/audio, recuperar recursos cuando corresponda y reiniciar delta para evitar saltos gigantes.
- Assets de APK son de lectura; guardados en directorio escribible de la aplicación. Resolver rutas mediante el servicio de assets, no suponiendo un cwd de escritorio.
- Sonido, suspensión, pantalla bloqueada y regreso al juego probados en dispositivo.
- `application_id`, `version_code`, iconos, orientación y ABI validados. Elegir minSdk/targetSdk al fijar toolchain conforme a requisitos vigentes de distribución; no congelar valores de tienda sin comprobarlos.
- APK debug para iteración. Export release consulta claves por ruta/entorno seguro; no copia contraseñas o keystores al proyecto, logs, plantilla ni paquete. Firma/notarización requieren credenciales del autor; no se realizan en esta fase de documentación.

## Aceptación end-to-end

Crear starter en una ruta con espacios; ejecutar; empaquetar; mover artefacto a máquina limpia y jugar. Repetir con asteroides. Comparar manifest/checksums para builds equivalentes, normalizando timestamps y orden; no prometer identidad binaria si firma/herramientas introducen variación. Android añade instalación, touch, pausa, audio y AAB. Una plataforma solo aparece como soportada tras cerrar ese recorrido.
