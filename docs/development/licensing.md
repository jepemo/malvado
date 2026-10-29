# Licencias y atribución

Malvado fue creado por Jeremías Pérez. La licencia elegida para el **código propio nuevo de Malvado 2** es Apache License 2.0; el texto está en [LICENSE](../../LICENSE). Los archivos C propios y ejemplos/pruebas nuevos indican `SPDX-License-Identifier: Apache-2.0` y su atribución. Esta decisión no cambia las licencias de dependencias ni de archivos históricos.

## Código histórico de Malvado 1

El rockspec 1.0 declara Apache 2.0, mientras que varios archivos Lua históricos indican GPL versión 3 o posterior. Conservar esos encabezados y el rockspec histórico mientras se verifica su autoría y las condiciones de su distribución. La elección de Apache para el código nuevo no modifica automáticamente los permisos ya otorgados sobre copias distribuidas. Antes de reutilizar o redistribuir código histórico bajo Apache, confirmar la titularidad y documentar expresamente esa decisión; las contribuciones de terceros requieren su autorización si sus derechos no permiten el cambio.

## Dependencias y referencias

Lua descargado durante la compilación mantiene su licencia MIT; raylib y sus dependencias conservan sus propias licencias. Las fuentes BennuGD de `docs/api/upstream/` son material de referencia bajo sus avisos originales. No añadir encabezados Apache a estos archivos externos ni retirar sus avisos. La distribución del runtime, SDK y juegos debe incorporar los avisos y textos exigidos por las dependencias efectivamente incluidas.

Revisar la procedencia y licencia de todos los assets, ejemplos históricos y nuevas dependencias antes de cada release. La distribución y cualquier futuro rockspec 2.0 deben reflejar el alcance de Apache 2.0 y los avisos externos.
