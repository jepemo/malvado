# Malvado: siguiente generación

Propuesta de migración, revisada el 29 de octubre de 2026. Estado: **primer corte del runtime C en desarrollo**; CLI y GUI están planificados.

El motor actual usa Lua/LÖVE. La siguiente generación propone C + raylib, Lua incluido como interfaz principal, procesos cooperativos y herramientas para crear y distribuir juegos. La investigación se basa en el código de Malvado `24d02aba2b08f805a046aa04dcc2e29243eaa00f`.

## Lectura recomendada

1. [Plan y decisiones](planning/roadmap.md).
2. [Arquitectura C/raylib/Lua y procesos](architecture/runtime.md) y [núcleo C y binding Lua oficial](architecture/c-first.md).
3. [Investigación DIV/Fénix/BennuGD](api/research.md).
4. [Inventario completo de exportaciones de los módulos de referencia](api/inventory.md), [catálogo JSON](api/catalog.json) y [procedencia](api/upstream.md).
5. [Perfiles de compatibilidad y trabajo pendiente](api/compatibility.md).
6. [Contrato inicial de la API Lua](api/lua-contract.md).
7. [Ejemplos y criterios de aceptación](examples/README.md).
8. [Diseño de malvado-cli y exportaciones](specs/cli.md).
9. [malvado-gui: alcance, ImGui y herramientas inspiradas en DIV](specs/gui.md).
10. [Recursos: FPG, PNG/JSON y paquetes ZIP](specs/assets.md).
11. [Versiones propuestas](versions/README.md).
12. [Documentación para GitHub](development/publishing.md).
13. [CI, calidad y releases con GitHub Actions](development/ci.md).
14. [Distribución del SDK, herramientas y LuaRocks](development/distribution.md).
15. [Estilo C](development/c-style.md) y [licencias y atribución](development/licensing.md).
16. [Documentación y ejemplos de las APIs C y Lua](development/api-documentation.md).

[La lista histórica de API](history/legacy-api.md) es orientativa; no representa una especificación verificada ni la API nueva. Los planes anteriores quedan en [history](history/README.md). Los ejemplos anteriores dependen de LÖVE; el [primer corte C](versions/2.0.0-alpha.1.md) incluye un ejemplo propio.

## Organización de la documentación

- `planning/`: roadmap y decisiones de entrega.
- `specs/`: propuestas de producto para assets, CLI y GUI.
- `architecture/`: diseño técnico del runtime.
- `api/`: investigación, contrato propuesto, catálogo y fuentes fijadas.
- `versions/`: hitos y estado de implementación por versión.
- `development/`: CI, releases y publicación de la documentación.
- `examples/`: planificación de ejemplos y aceptación.
- `history/`: documentos sustituidos conservados para consulta.
- `tools/`: generadores utilizados por la documentación.

Cada documento debe indicar si describe el comportamiento actual o una propuesta. Añadir documentos a la carpeta correspondiente y enlazarlos desde este índice.

## Organización del repositorio

La raíz contiene los puntos de entrada y configuración; `src/` alberga el runtime C, `malvado/` el código Lua/LÖVE histórico, `examples/` juegos de ejemplo, `tests/` pruebas; CMake descarga las fuentes de las dependencias en el directorio de compilación ignorado por Git. Los módulos futuros se agruparán por responsabilidad según el [roadmap](planning/roadmap.md), sin mover código antes de tener una separación real.

## Regenerar la referencia

Desde la raíz del repositorio, con Python 3.9 o posterior:

```sh
python3 docs/tools/fetch_api_sources.py
python3 docs/tools/catalog_api.py
```

La primera orden descarga las fuentes fijadas si faltan y verifica sus hashes; requiere red para la descarga, pero ninguna dependencia externa. El generador funciona sin red y comprueba los hashes antes de regenerar. La API de referencia y el contrato Lua son documentos distintos: una firma BennuGD no se transforma automáticamente en una firma Lua válida.
