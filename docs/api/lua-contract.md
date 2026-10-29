# Contrato inicial Lua de Malvado 2

**Diseño propuesto, todavía no implementado.** Estos nombres pertenecen a Malvado; no son firmas oficiales de BennuGD. API principal: `local mv = require("malvado")`. No se exportan globales salvo activación del shim legacy.

| Firma propuesta | Contrato inicial | Realización |
| --- | --- | --- |
| `mv.start(init, options?)` | Crea contexto/ventana y llama init una vez; ejecuta hasta cierre, exit o ausencia de procesos de juego | Runner/binding C |
| `mv.process(fn)` → constructor | Define tipo de proceso; `fn(self)` corre en una coroutine de la VM | C |
| `Constructor{...}` → pid | Instancia y copia campos; ejecuta en siguiente tick | C |
| `mv.frame()` | Suspende hasta siguiente tick; error fuera de proceso | `lua_yield` desde C |
| `mv.signal(pid, signal)` | Señal tipada; handle inexistente produce error en perfil moderno | C |
| `mv.kill(pid)` | Alias de señal kill, sin árbol implícito en perfil moderno | C |
| `mv.send(pid, value)` | Encola referencia Lua compartida; error por destino inválido o límite | C/registry |
| `self:recv()` → value/nil | Desencola sin bloquear; no admitir nil como payload | C/registry |
| `mv.set_proc(pid, values)` | Actualiza campos validados; campos de identidad no escribibles | C |
| `mv.key(name)` → bool | Estado sostenido del tick; nombres documentados, adaptados por backend | C |
| `mv.set_title(text)` | Cambia título; degradación documentada en Android | C/raylib |
| `mv.clear_screen(r,g,b,a?)` | Configura fondo persistente; canales enteros 0–255, alpha 255 por defecto | C |
| `mv.font(size,r?,g?,b?,a?)` → font | Fuente con ownership del contexto; color por defecto blanco | C |
| `mv.write(font,x,y,text,align?)` → text_id | Texto persistente; align por defecto izquierda/superior | C |
| `mv.change_text(id,text,x?,y?,font?)` | Cambia texto; argumentos omitidos conservan valor | C |
| `mv.delete_text(id)` | Baja explícita; comportamiento de ID ya borrado por cerrar en P0 | C |
| `mv.load_image(path)` → image | Asset relativo al proyecto, error de carga con ruta | C |
| `mv.rand(min,max)` → integer | Rango inclusivo; RNG propio con algoritmo/semilla fijados | C |
| `mv.time_seconds()` → number | Reloj monótono desde inicio; sustituye `love.timer.getTime` en demos | C |
| `mv.exit(code?)` | Solicita cierre ordenado, code=0; termina proceso actual de forma controlada | C |

`options`: ventana con `width`, `height`, `title` y `tick_rate`; defaults 800×600, Malvado y 60. La definición de perfiles, empaquetado y configuración CLI se unificará antes de implementar para no mantener dos fuentes de defaults.

Propiedades iniciales: `id` read-only, `x/y/z=0`, `angle=0` en grados, `size=1`, `graph=nil`, `fps=0` (cada tick), más campos de usuario. Describir antes de P3 `file/fpg/fpgIndex`, dimensiones, ancla y unidades de `delta`: no asignarles semántica Bennu automáticamente. En el perfil nuevo `delta` será tiempo lógico en segundos entre activaciones; en legacy documentar el comportamiento anterior y la diferencia.

## Ejemplo básico propuesto

```lua
local mv = require("malvado")
local Hello = mv.process(function(self)
  local font = mv.font(40, 255, 255, 255)
  local text = mv.write(font, self.x, self.y, "Hola, Malvado")
  while not mv.key("escape") do
    mv.frame()
  end
  mv.delete_text(text)
end)

mv.start(function()
  mv.clear_screen(25, 30, 45)
  Hello { x = 60, y = 80 }
end, { width = 800, height = 600, title = "Mi juego" })
```

## Shim legacy propuesto

`require("malvado.compat.legacy").import_globals()` instala `process`, `frame`, `font`, `write`, `key`, etc. Solo para migración; no inyectar globales al importar el namespace normal. El constructor continúa devolviendo PID como el core actual. El shim adapta nombres/argumentos; el trabajo de runtime sigue en C. No emula el namespace `love` completo: reemplazar llamadas directas en los ejemplos.
