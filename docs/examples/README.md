# Ejemplos como contrato del motor

Todos los snippets nuevos son propuestas, no programas ya ejecutables con el runtime actual. Los ejemplos reales actuales viven en `examples/` y requieren LÖVE. Mantenerlos hasta tener su reemplazo verificado, guardando trazas/capturas de referencia.

## Ejemplos C y Lua de primera clase

Cada capacidad compartida del motor tendrá ejemplos C con la API pública y Lua con el binding oficial; un mismo programa puede demostrar varias funciones. Las páginas enlazarán a los archivos reales y CI los ejecutará. Ver [documentación de APIs](../development/api-documentation.md). Priorizar hello, procesos/árbol/señales y mensajes; además de un juego mixto C/Lua para verificar que comparten scheduler y contexto. El ejemplo C headless debe compilar y ejecutarse sin Lua ni raylib; la variante gráfica utiliza el backend raylib. Los fragmentos y targets son propuestos hasta implementar la [arquitectura C](../architecture/c-first.md).

## Migración de los ejemplos existentes

| Ejemplo actual | Qué debe conservar | Trabajo necesario |
| --- | --- | --- |
| `examples/helloworld/main.lua` | Constructor de proceso, fuente, texto, Escape y cierre automático | Namespace/shim legacy, fuentes C y runner |
| `examples/demo/main.lua` | Menú, spritesheets, teclado, textos y fades | Recursos persistentes, animación, z y cadencia por proceso |
| `examples/space_asteroids/main.lua` | Juego completo, movimiento, balas y colisión | Eliminar llamadas `love.math.random`/`love.graphics.points`; mapear entrada y gráficos |
| `examples/bench/main.lua` | Creación de procesos, carga y tiempos | Sustituir `love.timer.getTime`; separar coste de VM, scheduler y render |

Los benchmarks comparan mismo equipo, número de procesos, recursos, VSync y FPS objetivo. Medir CPU por tick, tiempo de render, memoria y coste de creación/baja; fijar semilla y publicar resultados sin prometer una aceleración antes de medir.

## Colección nueva mínima

| Ejemplo | Funciones demostradas | Comprobación |
| --- | --- | --- |
| `hello` | start/process/frame/font/write | Se ve el saludo; Escape termina sin fugas |
| `process-family` | PID, creación, parentesco, kill-tree | Árbol y traza de activación esperados |
| `signals` | sleep/freeze/wakeup/kill | Contadores y visibilidad distinguen estados |
| `messages` | send/recv | FIFO, referencia compartida documentada, destino muerto |
| `timing` | tick fijo, cadencia por proceso | Sin deriva acumulada a distintos FPS de render |
| `sprites` | image, atlas, pivot, angle, size, z | Captura con rotación, escala, espejo y empate de z |
| `collision` | AABB y luego máscara | Contacto de bordes y tamaños diferentes |
| `text` | write/change/delete, alineación, UTF-8 | Marcador cambia sin recrear objetos cada tick |
| `input` | teclado, ratón, mando | Pulsación sostenida vs evento y hotplug |
| `audio` | efecto, música, volumen y pause | Reanudación sin duplicar canales |
| `scroll` | región y cámara | Escenario grande con HUD fijo |
| `fpg-import` | Importar una variante FPG a PNG+JSON | IDs no consecutivos, puntos, transparencia y paleta preservados |
| `atlas-import` | Subconjunto de atlas Aseprite PNG+JSON | Duraciones, offsets y rechazo explícito de opciones no soportadas |
| `gui-workflow` | Copia de starter/asteroids abierta desde GUI | Crear, ejecutar, error a línea, inspeccionar ticks y exportar con servicios CLI |
| `legacy-formats` | FPG/MAP/FNT/PAL cuando existan | Asset con controles/paleta conocidos |
| `asteroids` | Juego completo y exportación | Ciclo iniciar/jugar/perder/reiniciar en cada destino |
| `android-touch` | touch, Back, guardar y lifecycle | Juega tras bloquear/desbloquear dispositivo |
| `native-process` | callback C con estado explícito | Se suspende por scheduler sin pila C suspendida |

Cada carpeta tendrá README con controles/comportamiento, requisitos, comandos, assets con licencia, screenshot cuando ayude y salida/traza esperada si la tiene. Los ejemplos Lua tendrán `main.lua`; los C tendrán `main.c` y configuración de compilación. `malvado.toml` se añade cuando exista el formato de proyecto y corresponda. El README identifica perfiles y versión mínima. Fuente única para código mostrado en documentación: incluir fragmentos del ejemplo real en el sitio; no mantener una copia manual diferente.

## Esqueleto de ejemplo de mensajes propuesto

```lua
local mv = require("malvado")
local Receiver = mv.process(function(self)
  while true do
    local msg = self:recv()
    if msg then
      print(msg.kind)
      return -- termina y permite cerrar cuando no quedan procesos de juego
    end
    mv.frame()
  end
end)

mv.start(function()
  local pid = Receiver {}
  mv.send(pid, { kind = "hola" })
end)
```

Aceptación: imprime `hola` una vez. La cola mantiene el mensaje hasta que el receptor empiece en el tick siguiente. Añadir comprobación de FIFO con varios mensajes y eliminación de receptor con mensajes pendientes.

Los recorridos y fixtures de [GUI](../specs/gui.md) y [assets/FPG](../specs/assets.md) se incorporan cuando existan sus capacidades. `gui-workflow` es una prueba de aceptación del producto sobre los mismos juegos; no un nuevo motor ni una copia divergente de asteroides.

## No declarar un ejemplo migrado solo porque compila

Comprobar contratos de API por pruebas headless cuando sea posible; comprobar render y entrada en ejecución real. Las capturas permiten tolerancias visuales de fuentes/GPU. Los ejemplos Android deben poder jugarse mediante touch sin teclado externo. CI debe ejecutar los ejemplos sin ventana cuando tengan modo de traza y hacer smoke tests de ventana en los destinos disponibles.
