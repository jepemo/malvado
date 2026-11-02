# SDK and projects

The development SDK includes the `malvado` command, the Lua runtime, and the C libraries. It works on the macOS or Linux host for which it was built. Malvado 2 is in development, and its tools can change.

## Build and install the SDK

First, follow [Get started](getting-started.md) to install the build tools and get the source. From the source directory, build the SDK archive:

```sh
mise run package-sdk
cd build/sdk-package
```

This creates `malvado-sdk-alpha.4-dev.tar.gz` and its `.sha256` file. The task will not overwrite an existing archive. Move or remove the old files before you run it again.

Check the archive before you extract it. On Linux, run:

```sh
sha256sum -c malvado-sdk-alpha.4-dev.tar.gz.sha256
```

On macOS, run:

```sh
shasum -a 256 -c malvado-sdk-alpha.4-dev.tar.gz.sha256
```

Extract it and add the SDK to your path for this terminal:

```sh
mkdir -p "$HOME/malvado-sdk"
tar -xzf malvado-sdk-alpha.4-dev.tar.gz -C "$HOME/malvado-sdk"
export PATH="$HOME/malvado-sdk/bin:$PATH"
malvado --version
```

Use an empty directory for each SDK version. To use the command in a new terminal, add the `export PATH` line to your shell configuration or run it again.

## Create and run a game

Create a Lua game and run it:

```sh
malvado new my-game
cd my-game
malvado run
```

Edit `main.lua` to change the game. Run `malvado check` to check the syntax of the Lua files without starting the game.

To create a C game, use `malvado new --c my-c-game`. Run it with `malvado run` from the new directory. C projects need CMake and a C compiler. Use `malvado build` to build without starting the game.

## Export a game

From the project directory, run:

```sh
malvado export
./dist/game/game
```

The exported folder contains the game and its license notices. Copy the full folder to the target computer. The export is for the current host; cross-platform export is not available yet. The destination must not already exist. Use `malvado export ../another-folder` to choose a different destination.

Exported games skip hidden files, and directories named `build` or `dist`. They cannot include symbolic links. For the current limits and project rules, see the [CLI guide](https://github.com/jepemo/malvado/blob/master/docs/specs/cli.md).

Use `malvado --help` to list commands and `malvado doctor` to see the capabilities of your SDK. Graphics and system dependencies still need validation on more target machines.
