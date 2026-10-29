# Fuentes de referencia fijadas

Estos ficheros son originales externos descargables para investigación de API. No se versionan ni son dependencias del build de Malvado.

- BennuGD: `d49f71891f38db9bbf08c0f6f0002007f564a03c`.
- BennuGD2: `baf8dc1b744d47dae58fc20c139dd44656907911`.
- [Manifest con URL, ruta original y SHA-256](upstream-manifest.json).
- Avisos originales: [BennuGD modules/COPYING](https://github.com/SplinterGU/BennuGD/blob/d49f71891f38db9bbf08c0f6f0002007f564a03c/modules/COPYING), [BennuGD2 LICENSE.md](https://github.com/SplinterGU/BennuGD2/blob/baf8dc1b744d47dae58fc20c139dd44656907911/LICENSE.md) y encabezados de cada fichero. Conservarlos en cualquier redistribución; no atribuir este código a Malvado.

La descarga conserva la codificación original y los saltos de línea. El catálogo derivado es nuevo y puede consultarse sin descargar ni compilar las fuentes. Contiene metadatos de interfaz, no traducciones de las implementaciones.

## Reproducción

```sh
python3 docs/tools/fetch_api_sources.py
python3 docs/tools/catalog_api.py
```

El generador verifica hashes. Se incluyen todos los `*_exports.h` de los árboles consultados: 45 en BennuGD y 10 en BennuGD2. Los auxiliares no cuentan como cabeceras de exportación. Para actualizar, obtener primero el árbol y el SHA del nuevo commit, descargar todas sus exportaciones, actualizar el manifest y revisar el diff generado en un PR; nunca descargar desde una rama mutable sin fijar el resultado.

El generador es un extractor léxico, no un preprocesador C: incluye ramas alternativas. `preprocessor_context` conserva la secuencia de directivas por nivel, para interpretar `else/elif` sin fingir que son condiciones evaluadas. Las constantes conservan expresiones, no valores numéricos resueltos. En globales/locales, `raw_initializer` conserva el inicializador (sin comentarios) y sus macros; `declaration_string_fragments` concatena solo literales y **no expande macros**, por lo que no sustituye al original.

Las firmas de `FUNC` se guardan literalmente (`I`, `D`, `S`, `P`, `B`, etc.). Revisar la revisión correspondiente para traducir tipos, anchura y parámetros de salida. No asumir que un carácter equivale siempre a un argumento público Lua. Los aliases y sobrecargas siguen siendo filas independientes. Los bloques de variables se conservan completos; no se cuenta cada campo de una estructura como símbolo independiente.

Los auxiliares `sysprocs*` y `instance.c` son materiales para investigar el core, no una extracción completa de su contrato. No hay snapshot de todo DIV/Fénix ni de extensiones de terceros.
