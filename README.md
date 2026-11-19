# Malvado

Malvado is an open-source engine for making 2D games in C, Lua and experimental
Rust. Inspired by DIV Games Studio, it lets you give each character, object or
effect its own process. Malvado 2.0 is currently in beta.

[Documentation](https://jepemo.github.io/malvado/) · [Downloads](https://github.com/jepemo/malvado/releases)

## Hello, Malvado!

This Lua example draws a circle that you can move with **Left** and **Right**.
Press **Escape** to exit.

```lua
require("malvado").import_globals()

local Player = process(function(self)
  self.graph = circle(16, {color = {80, 190, 255}})

  while not key(KEY_ESCAPE) do
    if key(KEY_LEFT) then self.x = self.x - 3 end
    if key(KEY_RIGHT) then self.x = self.x + 3 end
    frame()
  end
end)

start(function()
  clear_screen(25, 30, 45)
  Player {x = 160, y = 120}
end)
```

`Player { ... }` creates a process with its own position. `frame()` lets
other processes run before the player continues.

## Run the examples

You need Git, mise and a C compiler to run the Hello World example. On Linux,
install the X11 and OpenGL development packages listed in the documentation.

```sh
git clone https://github.com/jepemo/malvado.git
cd malvado
mise install
mise run hello
```

Licensed under [Apache 2.0](LICENSE).
