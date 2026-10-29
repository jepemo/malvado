# Perfiles y matriz de compatibilidad

Estado: propuesta. Ninguna compatibilidad con un runtime nuevo está certificada.

## Perfiles independientes

- **malvado-legacy:** preservar DSL actual, grados, tamaño 1.0, claves de teclado de texto, textos y ejemplos existentes. Registrar cada cambio intencional en migración.
- **malvado-2:** API Lua con namespace, ownership seguro, errores consistentes y tipos C opacos. Es el perfil predeterminado propuesto.
- **bennu2-essential:** procesos/señales, gráficos 2D, texto, entrada, audio, reloj y utilidades priorizadas de la revisión fijada. Especificar cada símbolo antes de congelarlo.
- **bennu-classic-extended:** formatos, scroll, Mode 7, efectos, pathfinding y otras funciones históricas; ampliación por versiones.
- **div/fenix:** contraste histórico y adaptaciones explícitas. No se promete leer programas `.prg`, `.dcb` ni reproducir un compilador propio.

## Familias y secuencia de trabajo

| Familia | Destino | Ejemplo mínimo | Diferencia que verificar |
| --- | --- | --- | --- |
| Proceso, familia, señales y búsqueda | Core C + binding Lua | padre crea hijos, sleep/freeze/kill-tree | orden, hijo inmediato/diferido, retorno de collision/get_id |
| Ventana, reloj y regiones | Adaptador C + core | pantalla lógica escalada y dos regiones | frame vs render, DPI, clipping y cierre |
| Sprite, mapa, color, puntos, FPG | Recursos/render C | atlas, puntos de control, espejo, alpha | formatos binarios, paletas, pivotes y unidades |
| Dibujo y objetos persistentes | Módulo C | primitivas que se mueven y borran | ID persistente frente a dibujar una vez |
| Texto y fuentes | Módulo C | texto fijo, marcador actualizable, alineación | FNT, UTF-8, `write_var`, lifetime |
| Teclado, ratón, mando y touch | Snapshot de entrada C | movimiento, click, mando conectado | scancodes, repeat, deadzones, Android |
| Sonido/música | Módulo C/raylib | WAV y música con pausa | canales, bucles, códecs y ciclo de vida |
| Matemáticas, aleatorio y strings | C y stdlib Lua cuando equivalga | semilla fija y movimiento angular | miligrados, rangos y redondeo |
| Archivos, directorios y tiempo | C/plataforma | guardar partida y leer assets | rutas de escritura, empaquetado, epoch |
| Scroll/Mode 7/regiones | Render C propio | cámara sobre tilemap, suelo perspectivo | no son funciones raylib directas |
| Máscaras, pathfinding y efectos | Algoritmos C propios | nave contra contorno, laberinto | AABB vs píxeles, escalado/rotación |
| Regex, sort, memoria, criptografía | Adaptación por símbolo | búsqueda/orden con límites | punteros a memoria Bennu no son userdata Lua |
| FLIC/CD-ROM | Pendiente histórico | fixture mínimo si se implementa | decodificador/servicio específico o no soportado |
| Red/debug | Opcional posterior | echo local, inspector de procesos | raylib no proporciona el servicio de red |
| Ads/IAP | Extensión opcional por plataforma | mock de eventos y prueba con SDK | credenciales, tienda y lifecycle; fuera de 2.0 esencial |

## Formatos y herramientas

La compatibilidad de FPG se clasifica por variante y dirección: lectura, conversión y escritura son capacidades distintas. [La decisión de assets](../specs/assets.md) conserva ID/puntos/paleta y propone PNG+JSON editable sin eliminar el acceso histórico `file/graph`. La [GUI](../specs/gui.md) solo presenta importadores/exportadores que existan en los servicios C compartidos con CLI; un visor no certifica fidelidad ni soporte de todas las variantes.

## Registro de seguimiento por símbolo

El catálogo generado usa `status=not_assessed` para todas las entradas de referencia. No editarlo manualmente. Crear una capa de decisiones independiente, por ID estable `engine/module/name/signature/return_type` (añadir condición si hay variantes), con estos campos:

```json
{
  "id": "bennugd2/libmod_misc/SIGNAL/II/TYPE_INT",
  "profile": "bennu2-essential",
  "status": "planned",
  "lua_name": "malvado.signal",
  "implementation": null,
  "contract": "lua-contract.md",
  "example": "signals",
  "conformance_test": null,
  "divergences": ["Revisar selector por tipo y manejo de arboles"]
}
```

Estados: `not_assessed`, `specified`, `planned`, `implemented`, `verified`, `unsupported`, `deferred`. Motivo obligatorio para los dos últimos. La matriz se rellenará al implementar; la tabla anterior clasifica familias, no certifica todas sus funciones. Para un perfil «completo», cada entrada, sobrecarga, constante y variable incluida debe tener decisión y evidencia. Los campos del core se añaden como entradas propias.

## Formato de ficha de API

Nombre y aliases; procedencia/perfil; firma Lua; contrato C; argumentos con unidades/rango/default; retornos/errores; ownership; estado global o local afectado; disponibilidad por plataforma; ejemplo ejecutable; divergencias; prueba de conformidad. Documentar códigos históricos de error en el perfil de compatibilidad y errores Lua en el perfil moderno.
