# Malvado

Malvado is a game engine inspired by DIV Games Studio. Write games in Lua with simple processes, or use the C API directly.

> Malvado 2 is in development. The API and tools can change.

## Hello World

A process is a part of your game that runs over time. This one shows a greeting until you press Escape:

```lua
require("malvado").import_globals()

local HelloWorld = process(function(self)
  local font = font(60, 0, 0, 0)
  write(font, self.x, self.y, self.text)

  while not key("escape") do
    frame()
  end
end)

start(function()
  set_title("Hello world")
  clear_screen(250, 250, 250)
  HelloWorld { x = 400, y = 300, text = "Hello World" }
end)
```

`process` defines the behavior. `HelloWorld { ... }` starts it with its own values. `frame()` lets the rest of the game run before the process continues.

## Get started

Install [Git](https://git-scm.com/), [mise](https://mise.jdx.dev/getting-started.html), and a C compiler. On Linux, also install the X11 and OpenGL development packages listed in [CI](.github/workflows/ci.yml).

Get the source and run Hello World:

```sh
git clone https://github.com/jepemo/malvado.git
cd malvado
mise install
mise run hello
```

The first build needs network access to download and verify the pinned dependencies. The example needs a display. Press **Escape** or close the window to exit.

## Try more examples

After the build, run these commands from the project directory:

```sh
./build/malvado-run examples/demo/main.lua
./build/malvado-run examples/space_asteroids/main.lua
./build/malvado-run examples/bench/main.lua
```

See the instructions for [Hello World](examples/helloworld/README.md), the [demo](examples/demo/README.md), [Space Asteroids](examples/space_asteroids/README.md), and the [benchmark](examples/bench/README.md). The [C and Lua process examples](examples/c/processes/README.md) show both APIs.

## Documentation and development

Read the [documentation](https://jepemo.github.io/malvado/) for more guides. The [alpha.3 testing guide](docs/versions/2.0.0-alpha.3-testing.md) lists the current examples and checks. See the [version plan](docs/versions/README.md) for the project status.

Run `mise tasks` to see the development commands. Use `mise run test` for the standard tests, `mise run test-headless` to test Lua without a graphics backend, and `mise run test-c` to test the C core without Lua or raylib. `mise run test-visual` needs a display or virtual display.

CMake also works without mise. The [runtime guide](docs/versions/2.0.0-alpha.1.md) explains build options and how to use local dependency sources.

## License

Malvado 2 uses the [Apache License 2.0](LICENSE). Dependencies keep their own licenses. See [licenses and attribution](docs/development/licensing.md) for details.
