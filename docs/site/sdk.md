# SDK and projects

The development SDK includes the `malvado` command, the Lua runtime, and the C libraries. It works on the macOS or Linux host for which it was built. Malvado 2 is in development, and its tools can change.

## Install a local development build

From a source checkout on macOS or Linux, run:

```sh
mise run install
export PATH="$HOME/.local/share/malvado-dev/bin:$PATH"
malvado --version
```

The task builds and installs a development SDK for your user. Run `mise run install` again after you change the source. It replaces the managed SDK. Run `mise run uninstall` to remove it. The tasks do not change your shell configuration. Add the `export PATH` line to your shell configuration to use the command in new terminals.

Set `MALVADO_DEV_PREFIX` to choose another installation directory. Add its `bin` directory to `PATH` instead. Use that directory only for the development SDK. The tasks refuse to change an unmanaged directory or a directory with extra files. They do not remove your projects.

## Build and install the SDK

First, follow [Get started](getting-started.md) to install the build tools and get the source. From the source directory, build the SDK archive:

```sh
mise run package-sdk
cd build/sdk-package
```

This creates an archive and its `.sha256` file. The archive name includes the version, operating system, and architecture. The task will not overwrite an existing archive. Move or remove the old files before you run it again. The commands below assume that this directory contains only one SDK archive.

The extracted SDK also records its version and target in `share/malvado/malvado-sdk-info.txt`.

Check the archive before you extract it. On Linux, run:

```sh
sha256sum -c malvado-sdk-*.tar.gz.sha256
```

On macOS, run:

```sh
shasum -a 256 -c malvado-sdk-*.tar.gz.sha256
```

Extract it and add the SDK to your path for this terminal:

```sh
mkdir -p "$HOME/malvado-sdk"
tar -xzf malvado-sdk-*.tar.gz -C "$HOME/malvado-sdk"
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

The new Lua project returns a game table. Its `init` function starts the game processes. Set `title`, `width`, and `height` in the table to change the window. For a small example, see the [declarative game](https://github.com/jepemo/malvado/tree/master/examples/declarative). You can also use `mv.start()` in an entry script. Use one entry form per run.

To create a C game, use `malvado new --c my-c-game`. Run it with `malvado run` from the new directory. C projects need CMake and a C compiler. Use `malvado build` to build without starting the game.

## Export a game

From the project directory, run:

```sh
malvado export
./dist/game/game
```

The exported folder contains the game and its license notices. Copy the full folder to the target computer. The export is for the current host; cross-platform export is not available yet. The destination must not already exist. Use `malvado export ../another-folder` to choose a different destination.

For a Lua game, the launcher loads `main.lua` from the exported folder. It supports both a returned game table and `mv.start()`.

On Linux, you can create an archive to share the exported folder:

```sh
tar -czf my-game.tar.gz -C dist game
```

The recipient can extract the archive and run `./game/game`.

On macOS, you can instead create an application bundle with `malvado export --app`. It creates `dist/game.app`. To choose a different name and destination, use a path that ends in `.app`, such as `malvado export --app ../my-game.app`. The name must be valid UTF-8 without control characters. This initial bundle uses a default identifier. It is not signed or notarized by Malvado. Manual clean-machine validation is planned for beta.

To give a game its own macOS identifier, use `malvado export --app --id org.example.my-game`. Replace the example with an identifier for your game. Without `--id`, every app uses `org.malvado.game`.

To share a macOS app, select the `.app` in Finder and choose **Compress**. Send the resulting ZIP file. The recipient can extract it and open the app.

Exported games skip hidden files, and directories named `build` or `dist`. They cannot include symbolic links. For the current limits and project rules, see the [CLI guide](https://github.com/jepemo/malvado/blob/master/docs/specs/cli.md).

Use `malvado --help` to list commands and `malvado doctor` to see the capabilities of your SDK. Graphics and system dependencies still need validation on more target machines.
