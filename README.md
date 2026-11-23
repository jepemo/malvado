# Malvado

Malvado is an open-source engine for making 2D games in C, Lua and experimental
Rust. Inspired by DIV Games Studio, it lets you give each character, object or
effect its own process. Malvado 2.0.0 is released for macOS arm64 and Linux
x86_64; Windows x64 and Android arm64 keep beta platform support.

[Documentation](https://malvado.dev/) · [Downloads](https://github.com/jepemo/malvado/releases)

## Install Malvado

Install the SDK on macOS arm64 or Linux x86_64 with one command:

```sh
curl -fsSL https://malvado.dev/install.sh | bash
```

Then create and run a game:

```sh
malvado new my-game
cd my-game
malvado run
```

The installer verifies the release checksum and needs no compiler or repository
checkout. On Windows, download the SDK archive from the
[releases page](https://github.com/jepemo/malvado/releases). See
[Get started](docs/site/getting-started.md) for the full instructions.


## Hello, Malvado!

Malvado 2 uses a procedural Lua API inspired by DIV Games Studio. Managed
resource handles are passed to functions rather than manipulated through
instance methods.

This Lua example draws a circle that you can move with **Left** and **Right**.
Press **Escape** to exit.

```lua
require("malvado").import_globals()

local Player = process(function(self)
  self.graph = circle {radius = 16, color = BLUE}

  while not key(KEY_ESCAPE) do
    if key(KEY_LEFT) then self.x = self.x - 3 end
    if key(KEY_RIGHT) then self.x = self.x + 3 end
    frame()
  end
end)

start(function()
  clear_screen(BLACK)
  Player {x = 160, y = 120}
end)
```

`Player { ... }` creates a process with its own position. `frame()` lets
other processes run before the player continues.

## Build from source (contributors)

To build Malvado from source, you need Git, mise and a C compiler. On Linux,
install the X11 and OpenGL development packages listed in the documentation.

```sh
git clone https://github.com/jepemo/malvado.git
cd malvado
mise install
mise run hello
```

Licensed under [Apache 2.0](LICENSE).
