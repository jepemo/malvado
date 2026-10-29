# Documentar Malvado en GitHub

El detalle de workflows, permisos y generación de releases está en [CI y GitHub Actions](ci.md). El método concreto para documentar las APIs C y Lua con ejemplos se define en [documentación de APIs](api-documentation.md).

## Propuesta

Usar **Markdown en `docs/` como fuente**, revisado en los mismos PR que el código, y publicar un sitio estático en GitHub Pages cuando exista el primer corte ejecutable. No usar GitHub Wiki como fuente principal: separa la documentación del versionado y de los cambios del motor.

Para el sitio se propone MkDocs con el tema por defecto, con versión fijada al implementarlo. Las fuentes públicas irán en `docs/site/`; los planes y snapshots históricos permanecen fuera del directorio publicado. La documentación seguirá siendo legible directamente en GitHub sin generar nada. No se instala ni se publica un sitio en esta fase.

GitHub admite [publicar generadores estáticos mediante workflows de Pages](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages). El workflow futuro debe validar y construir en PR; desplegar únicamente desde la rama/release configurada. No activar despliegue automático por añadir estos documentos.

## Tres referencias relacionadas

1. **Guía de usuario:** instalación, primer juego, procesos, assets, controles, CLI, exportación y solución de problemas.
2. **API Lua:** fichas del contrato público, tipos/ownership, ejemplos y disponibilidad. Los juegos deben poder aprenderse desde aquí.
3. **API C:** referencia por módulo con firmas, ownership, errores y ejemplos C.
4. **Compatibilidad:** inventario Bennu, perfiles, divergencias y enlaces a código fuente fijado.

LDoc también puede documentar módulos Lua escritos en C, pero necesita anotaciones explícitas para sus funciones; no se adopta de entrada. Las cabeceras públicas C y definiciones LuaCATS para el editor serán fuentes de firmas por lenguaje; Markdown explicará el comportamiento y enlazará a ejemplos reales. Añadir Doxygen o LDoc tras evaluar su utilidad, sin exigir inicialmente un esquema JSON propio. El catálogo Bennu no es la API propia.

## Organización del sitio futuro

Inicio → Primer juego → Tutoriales → Procesos y temporización → Gráficos/texto/entrada/audio → CLI y exportación por plataforma → malvado-gui y diagnóstico de procesos → Recursos/FPG/atlas/ZIP → API Lua → API C → Migrar desde LÖVE/Malvado 1 → Perfiles DIV/Bennu → Desarrollo y versiones.

El inventario externo extenso debe quedar en la sección de compatibilidad, sin eclipsar la guía de creación de juegos. Mantener docs de usuario en español inicialmente; decidir inglés/traducciones cuando haya capacidad para mantener ambas. API y símbolos conservan sus nombres originales.

## Ejemplos y generación

- Mostrar fragmentos desde `examples/`, con enlace al juego completo, controles y screenshot/GIF pequeño cuando ayude.
- Un símbolo estable necesita al menos un ejemplo reproducible y versión mínima.
- CI regenera el catálogo y falla si difiere de lo guardado; verifica hashes, enlaces relativos y estructura de fichas.
- Las sobrecargas externas y los estados de implementación se gestionan por separado: regenerar inventario no borra decisiones manuales.
- Doxygen y LuaLS se añaden cuando haya cabeceras/contrato propios; la documentación C no reemplaza los tutoriales Lua.
- Ejecutar ejemplos headless donde sea posible; no simular que un build de Markdown valida un juego con ventana.

## Documentación de herramientas y recursos

La [GUI](../specs/gui.md) tendrá guía por tareas, controles, capturas y versiones mínimas de runtime/protocolo. Distinguir Step tick de depuración de líneas y cambios temporales en ejecución de cambios guardados en assets. El mismo tutorial starter→asteroids debe poder completarse por CLI y GUI.

La [guía de formatos](../specs/assets.md) publica matriz FPG por variante, importación frente a exportación, dialectos de atlas soportados y significado del paquete ZIP. Nunca anunciar «compatible FPG» sin límites y fixtures. La documentación de UI/ImGui es de desarrollo; el usuario necesita entender sus recursos y errores de conversión.

## Versionado y proyecto

README raíz breve con inicio rápido, enlaces al sitio y soporte real de plataformas. `docs/versions/` recoge propuestas y aceptación; CHANGELOG registra entregas. Publicar `/dev/` y `/2.0/` cuando el sitio exista; no duplicar a mano todos los documentos por parche.

GitHub Issues por entregable con enlace al contrato/ejemplo; milestones para alphas/beta; un Project opcional para estados. Las issues no sustituyen la especificación versionada. Releases con SDKs/runtimes por destino, checksums, avisos de licencia y enlaces a la documentación de esa versión. El trabajo actual no crea issues, tags, releases ni publicaciones externas.
