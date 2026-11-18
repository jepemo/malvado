# First Malvado lessons

Start a game, run two processes, then load and draw an image. Each lesson
has versions in C, Lua and Rust.

1. [Startup](startup/README.md)
2. [Processes](processes/README.md)
3. [Image](image/README.md)

## Prepare a project

Install a matching graphical desktop SDK and put its `bin` directory on
`PATH`. C and Rust projects need CMake and a C compiler. Rust also needs
Cargo and rustc. See [SDK and projects](../../docs/site/sdk.md).

Set `source` to the absolute path of the lesson directory in your Malvado
checkout. From a separate directory, choose one language and run its commands.
Use a fresh project directory. The starter creates the build configuration
for your installed SDK.

```sh
# C
malvado new --c lesson
cp "$source/c/main.c" lesson/main.c
```

```sh
# Lua
malvado new lesson
cp "$source/lua/main.lua" lesson/main.lua
```

```sh
# Rust
malvado new --lang rust lesson
cp "$source/rust/main.rs" lesson/main.rs
```

Follow the lesson's remaining steps. To share it on the same desktop platform,
run `malvado export` from the project directory. Run `./dist/game/game` on
macOS/Linux or `.\dist\game\game.exe` in PowerShell. For the headless Lua
lessons, pass `--headless` to the exported launcher.

## Verify the lessons

Use an installed desktop SDK and a display:

```sh
python3 tools/test_beta_examples.py --sdk "/path/to/installed sdk"
```

The verifier checks each lesson's output from the installed SDK and exported
game. It checks that the exports contain the shared PNG and project license.
On Linux X11, pass `--pixels` to verify the exported tile and background
with `xdotool` and ImageMagick. CI runs this check under Xvfb.
