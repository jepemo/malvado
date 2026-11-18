# Get started

Build Malvado and run the Hello World example. It opens a window and shows a greeting.

Use C, Lua or experimental Rust. If you already have a matching development
SDK, go to [Create a project](#create-a-project).

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

## Create a project

Install a matching SDK and make `malvado` available in your terminal. See
[SDK and projects](sdk.md) for macOS, Linux and Windows installation.
Run the command for your language from a directory where you keep projects.

| Language | Command | Tools for building the game |
| --- | --- | --- |
| Lua | `malvado new my-game` | The SDK includes the Lua runner. |
| C | `malvado new --c my-game` | CMake and a C compiler. |
| Rust | `malvado new --lang rust my-game` | Cargo, rustc, CMake and a C compiler. |

```sh
cd my-game
malvado check
malvado run
```

The starter opens a window. Close it with Escape or the window's close control.
Edit `main.lua`, `main.c` or `main.rs` to change the greeting. C and Rust
projects can use `malvado build` to compile before running. See the
[gameplay API guides](api.md) for the scheduler, resources and language-specific
entry points.

## Try more examples

Run `mise tasks` to see the available commands. Use `mise run test-headless` to test Lua without a graphics backend. Use `mise run test-c` to test the C core without Lua or raylib.

Try the [first three lessons](https://github.com/jepemo/malvado/blob/master/examples/features/README.md)
to start a game, run two processes and draw a PNG. Each lesson has C, Lua and
Rust versions with the same behavior.
