# Get started

Malvado 2 uses a procedural Lua API inspired by DIV Games Studio. Managed
resource handles are passed to functions rather than manipulated through
instance methods.

Install the Malvado SDK and run your first game. The installer downloads a
prebuilt SDK; no compiler or repository checkout is needed. Use C, Lua or
experimental Rust.

## Install the SDK

The installer supports macOS arm64 and Linux x86_64:

```sh
curl -fsSL https://malvado.dev/install.sh | bash
```

It downloads the release SDK, verifies its SHA-256 checksum and installs it
under `~/.local/share/malvado`. It then prints the line that adds the `bin`
directory to your `PATH`. Add that line to your shell configuration to keep
the `malvado` command.

On Windows, download the SDK archive from
[GitHub Releases](https://github.com/jepemo/malvado/releases).

## Uninstall the SDK

To remove an SDK installed with the script, run:

```sh
curl -fsSL https://malvado.dev/install.sh | bash -s -- --uninstall
```

If you installed with `--prefix`, pass the same `--prefix` when you uninstall.
If you added the SDK to your `PATH`, remove that entry from your shell
configuration.

## Create and run a game

Run the command for your language from a directory where you keep projects:

| Language | Command | Tools for building the game |
| --- | --- | --- |
| Lua | `malvado new my-game` | The SDK includes the Lua runner. |
| C | `malvado new --lang c my-game` | CMake and a C compiler. |
| Rust | `malvado new --lang rust my-game` | Cargo, rustc, CMake and a C compiler. |

```sh
cd my-game
malvado check
malvado run
```

The starter opens a window. Close it with **Escape** or the window's close
control. Edit `main.lua`, `main.c` or `main.rs` to change the greeting. C and
Rust projects can use `malvado build` to compile before running. See
[SDK and projects](sdk.md) for the SDK and [gameplay API guides](api.md) for the
scheduler, resources and language entry points. The Rust starter uses a
named async process and the managed runner. See the
[first Rust games](https://github.com/jepemo/malvado/tree/main/examples/rust)
for Hello World and several independently moving balls.

## Lua in VS Code

For a Lua project, open its folder in VS Code and install the
[Lua Language Server extension](https://marketplace.visualstudio.com/items?itemName=sumneko.lua)
(`sumneko.lua`). The `.luarc.json` created by `malvado new` gives you
autocompletion and type checking for Malvado's Lua API.
See the [Lua Language Server guide](https://github.com/jepemo/malvado/blob/main/docs/api/lua-language-server.md)
for existing projects.

## Build from source (contributors)

To build Malvado from source, install [Git](https://git-scm.com/),
[mise](https://mise.jdx.dev/getting-started.html) and a C compiler. On Linux,
install the X11 and OpenGL development packages listed in the
[CI workflow](https://github.com/jepemo/malvado/blob/main/.github/workflows/ci.yml).

Clone the source and install the project tools:

```sh
git clone https://github.com/jepemo/malvado.git
cd malvado
mise install
```

The first build needs network access. CMake downloads the pinned Lua, raylib and
cJSON sources and checks their hashes. Run the tests, then start the Hello World
example:

```sh
mise run test
mise run hello
```

Press **Escape** or close the window to exit. The example needs a display. Read
its [source and instructions](https://github.com/jepemo/malvado/tree/main/examples/helloworld).

## Try more examples

Run `mise tasks` to see the available commands. Use `mise run test-headless` to
test Lua without a graphics backend. Use `mise run test-c` to test the C core
without Lua or raylib.

Try the [first three lessons](https://github.com/jepemo/malvado/blob/main/examples/features/README.md)
to start a game, run two processes and draw a PNG. Each lesson has C, Lua and
Rust versions with the same behavior.
