# Recursos gráficos: FPG, atlas modernos y paquetes ZIP

Estado: decisión propuesta, no soporte implementado. Completa el análisis de formatos del plan y de malvado-gui.

## Decisión recomendada

**Soportar FPG como vía de compatibilidad e importación; usar PNG y metadatos JSON como recursos editables; usar ZIP cuando convenga transportar o distribuir varios archivos.** Son capas diferentes: ZIP es un contenedor, no define gráficos, puntos, animaciones ni IDs.

No crear un nuevo formato binario de imagen. Sí hace falta un esquema pequeño de metadatos de Malvado, público y versionado, porque sus puntos de control, IDs históricos y perfiles de colisión no tienen una representación universal. Reutilizar formatos de intercambio existentes mediante adaptadores; no afirmar que «JSON» por sí solo sea un estándar único de atlas.

## Qué representa FPG y qué conservar

FPG es una colección gráfica de la familia DIV, distinta de un único spritesheet y de un paquete completo de juego. El [lector/escritor de BennuGD clásico](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/mod_map/file_fpg.c) conserva códigos numéricos, nombres, dimensiones, gráficos y puntos de control; contempla variantes de profundidad y paletas. Esto hace útil importar material existente sin perder sus referencias. No asumir que todas las variantes de DIV/Fénix/BennuGD tienen el mismo layout.

La [implementación de BennuGD2 consultada](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/modules/libmod_gfx/m_fpg.c) carga FPG, mientras su wrapper de guardado está comentado en esa revisión. Por tanto, leer FPG y escribir FPG deben aparecer como capacidades diferentes; no prometer exportación porque una variante histórica la tuvo.

Malvado actual llama `fpg()` a directorios/spritesheets y tiene experimentos ZIP en `malvado/map.lua`; eso no demuestra soporte binario FPG. El perfil legacy debe distinguir esa API del lector real.

## Comparación de opciones

| Opción | Uso adecuado | Ventajas | Límites |
| --- | --- | --- | --- |
| FPG histórico | Reutilizar gráficos/proyectos DIV y derivados | IDs y puntos conocidos por ese ecosistema | Variantes, paletas, binario difícil de revisar; no es un proyecto completo |
| PNG individuales + JSON | Fuente editable y primer runtime | Legible, Git, herramientas externas y carga simple | Muchos archivos; metadatos con contrato propio explícito |
| Atlas PNG + JSON de herramienta | Intercambio y optimización de dibujo | Agrupa regiones, admite pipeline externo | Dialectos diferentes, recorte/rotación y puntos requieren adaptación |
| ZIP + manifiesto | Compartir bibliotecas o distribuir juego | Contenedor conocido y varios tipos de recurso | Requiere lector C y límites; comprimir no equivale a proteger código |
| Binario propietario nuevo | Caché optimizada si hay mediciones que lo justifiquen | Podría reducir tiempos de carga | Complejidad, conversores, debugging y versionado; diferido |

[Aseprite permite exportar spritesheets PNG y metadatos JSON](https://www.aseprite.org/docs/cli/), con variantes json-array/json-hash y datos de animación. Proponer inicialmente un adaptador para exportaciones PNG+JSON json-array con un subconjunto probado; informar campos no soportados en lugar de ignorarlos. No requiere que el jugador tenga Aseprite ni redistribuir su editor.

Los mapas de niveles son otro recurso: [Tiled publica un formato JSON de mapas y tilesets](https://doc.mapeditor.org/en/stable/reference/json-map-format/). Considerar su importador cuando exista soporte de tilemaps; no forzar un nivel dentro de un FPG ni prometer compatibilidad Tiled completa en 2.0. PNG+JSON es una elección de pipeline, no una afirmación de que haya un único estándar moderno que sustituya todo DIV.

## Modelo C común a todos los importadores

Biblioteca de gráficos → entradas con ID lógico estable → imagen CPU/región de atlas → textura GPU. Conservar nombre, tamaño original, offsets de recorte, pivote, puntos de control, duración y secuencias de animación, filtrado y datos de colisión cuando existan. Los datos ausentes tienen defaults definidos; no inventar puntos de control a partir de nombres de archivos sin declararlo.

Los IDs numéricos FPG no se renumeran al ordenar carpetas o empaquetar un atlas. La API de compatibilidad `file/graph` usa una tabla de resolución a recursos del core; en API moderna pueden usarse nombres/handles. Los puntos se expresan en píxeles del espacio original del sprite, origen superior izquierdo; al recortar se aplica offset, al renderizar se transforma. Validar por separado índices históricos de puntos y el pivote utilizado para dibujo.

Para FPG indexado, conservar índices y paleta si el perfil permite cambios de paleta en ejecución. Convertir solo a RGBA puede preservar aspecto inicial pero perder ese comportamiento: el conversor debe avisar y documentarlo. Definir transparencia y valores de punto no definido con fixtures. No garantizar round-trip binario; el objetivo de importación es fidelidad de píxeles y metadatos soportados.

Ejemplo **ilustrativo** de metadatos Malvado, no JSON Aseprite ni esquema ya congelado:

```json
{
  "schema": 1,
  "kind": "graphics-library",
  "sprites": [
    {
      "id": 10,
      "name": "ship",
      "image": "ship.png",
      "pivot": { "x": 16, "y": 16 },
      "points": [{ "index": 1, "name": "muzzle", "x": 16, "y": 0 }]
    }
  ]
}
```

El tamaño se obtiene de la imagen; validarlo al importar. La convención del pivote y puntos debe adaptarse expresamente al perfil histórico. El esquema de biblioteca, el de proyecto TOML y el del paquete de distribución tienen versiones independientes. GUI y CLI comparten validadores y conversores C.

## Fuente editable y distribución

Durante desarrollo, carpeta normal con PNG/JSON/Lua y configuración TOML. Cambiar un punto o duración desde GUI modifica JSON con undo/redo y guardado atómico; las fuentes del artista permanecen aparte. La caché generada no sustituye las fuentes y queda fuera de Git.

Export: primero validar/cargar recursos y resolver referencias; después generar atlas si está activado; finalmente empaquetar runtime y recursos. Empezar con export de carpeta validado por destino. El ZIP de entrega puede contener esa carpeta. Un ZIP de assets montable por el runtime es una capacidad adicional y posterior, con manifiesto definido: no confundir el archivo descargable con el formato que abre el juego.

No imponer una extensión nueva como `.fpg2` o `.malvado` antes de necesitarla. Una eventual extensión de paquete documentará que contiene ZIP y un esquema, sin fingir ser FPG. En Android los recursos van en assets del APK, ya empaquetado; no añadir ZIP anidado por defecto ni suponer acceso mediante rutas de escritorio. El servicio de assets oculta las diferencias de origen, no el formato de contenido.

ZIP no cifra el juego y PNG ya está comprimido. Medir tamaño/carga antes de elegir compresión de todo; priorizar lecturas acotadas y no extraer indiscriminadamente. Seleccionar una biblioteca C de ZIP con licencia revisada en P0/P3; raylib no se considera por sí solo un sistema completo de paquetes.

## Secuencia de implementación

| Hito | Capacidad | Criterio de aceptación |
| --- | --- | --- |
| Core alpha.3 | PNG, directorios/spritesheets y modelo de biblioteca JSON básico | sprites/demo sin LÖVE; IDs estables y pivot probado |
| Core alpha.4 | Export de carpeta y ZIP de entrega | Juego funciona desde paquete extraído, mismas referencias que desarrollo |
| Core alpha.5 | Primer importador FPG de variante fijada; adaptador Aseprite básico | Fixture 8-bit elegido con IDs/puntos/paleta preservados; spritesheet con animación equivalente |
| GUI G1 | Explorar y previsualizar recursos ya soportados | No ofrece import FPG antes de existir lector |
| GUI G3 | Editar metadatos y convertir FPG soportado a PNG+JSON | Guardar/reabrir y comparar puntos, IDs y píxeles |
| 2.1+ | Variantes FPG adicionales, export FPG, ZIP montable, packing y formatos de mapas | Matriz de variantes y límites con pruebas propias |

La primera variante FPG se seleccionará con archivos de muestra y procedencia conocida, sin declarar «todos los FPG». Export FPG será opcional por variante: rechazar o pedir conversión explícita si hay datos no representables, sin cuantizar paletas o perder alpha silenciosamente. El perfil estable documenta qué cargas/conversiones existen y qué sigue diferido.

## Ejemplos y comprobaciones

Crear `fpg-import`: pequeña biblioteca con IDs no consecutivos, nombre, punto definido y no definido, paleta y transparencia; importar, generar PNG+JSON y mostrar la misma figura en runtime. Comparar píxeles/índices y metadatos soportados. Añadir variantes 16/32-bit solo con sus propios fixtures.

Crear `atlas-import`: export Aseprite con varios frames/duraciones, recorte y pivote; validar subset y errores claros para rotación u otras opciones no soportadas. El packing inicial puede evitar rotación; su importador no debe aceptar datos que luego dibuje mal.

Comprobar límites de dimensiones, overflow, tamaños truncados, IDs duplicados y versiones desconocidas. Para ZIP: rutas `../`, absolutas, duplicados con distinto case, tamaño descomprimido y número de entradas. Estas verificaciones pertenecen al lector/conversor C compartido, no solo al diálogo GUI. El fixture FPG debe tener licencia para redistribución; no copiar paquetes de juegos comerciales para probar.
