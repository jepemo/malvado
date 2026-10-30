# Get started

Build Malvado and run the Hello World example. It opens a window and shows a greeting.

Malvado 2 is in development. Its API and tools can change.

## Prepare your computer

Install [Git](https://git-scm.com/), [mise](https://mise.jdx.dev/getting-started.html), and a C compiler. On Linux, install the X11 and OpenGL development packages listed in the [CI workflow](https://github.com/jepemo/malvado/blob/master/.github/workflows/ci.yml).

Get the source and install the project tools:

```sh
git clone https://github.com/jepemo/malvado.git
cd malvado
mise install
```

The first build needs network access. CMake downloads the pinned Lua, raylib, and cJSON sources and checks their hashes.

## Run Hello World

Run the tests, then start the example:

```sh
mise run test
mise run hello
```

Press **Escape** or close the window to exit. The example needs a display. You can read its [source and instructions](https://github.com/jepemo/malvado/tree/master/examples/helloworld).

## Try more examples

Run `mise tasks` to see the available commands. Use `mise run test-headless` to test Lua without a graphics backend. Use `mise run test-c` to test the C core without Lua or raylib.

Try the [demo](https://github.com/jepemo/malvado/tree/master/examples/demo), [Space Asteroids](https://github.com/jepemo/malvado/tree/master/examples/space_asteroids), or the [C process example](https://github.com/jepemo/malvado/tree/master/examples/c/processes). Each example has its own instructions.
