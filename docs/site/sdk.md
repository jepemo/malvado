# SDK and projects

The SDK includes the `malvado` command, the Lua runtime, the C libraries,
and experimental Rust crate sources. See the [installation
instructions](getting-started.md#install-the-sdk) for a released desktop SDK.

## Install a local development build

From a source checkout on macOS or Linux, run:

```sh
mise run install
export PATH="$HOME/.local/share/malvado-dev/bin:$PATH"
malvado --version
```

The task builds and installs a development SDK for your user. Run `mise run install` again after you change the source. It replaces the managed SDK. Run `mise run uninstall` to remove it. The tasks do not change your shell configuration. Add the `export PATH` line to your shell configuration to use the command in new terminals.

Set `MALVADO_DEV_PREFIX` to choose another installation directory. Add its `bin` directory to `PATH` instead. Use that directory only for the development SDK. The tasks refuse to change an unmanaged directory or a directory with extra files. They do not remove your projects.

## Windows SDK prefix

Install the VS 2022 C++ desktop workload and Windows SDK, plus CMake and the
Rust toolchain when developing Rust games. From PowerShell in a source checkout:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel
cmake --install build --config Release --prefix "C:/Malvado SDK"
& "C:/Malvado SDK/bin/malvado.exe" doctor
```

Use that executable directly or add its `bin` directory to your PATH. The Unix
`mise run install` manager is not a Windows installer. Windows folder exports
start with `game.exe`; native C/Rust players include `game-bin.exe`. Players need
a working OpenGL 3.3/WGL provider and the Microsoft release CRT, but no Cargo,
rustc or compiler. Software-rendered CI and fresh-player checks are recorded in
the [platform matrix](https://github.com/jepemo/malvado/blob/main/docs/development/platforms.md).
Windows app bundles, cross exports and Unicode project/SDK paths are not claimed.
Windows x64 platform support is beta in Malvado 2.0.

## Build and install the SDK

First, follow [Get started](getting-started.md) to install the build tools and get the source. From the source directory, build the SDK archive:

```sh
mise run package-sdk
cd build/sdk-package
```

This creates an archive and its `.sha256` file. The archive name includes the version, operating system, and architecture. The task will not overwrite an existing archive. Move or remove the old files before you run it again. The commands below assume that this directory contains only one SDK archive.

The extracted SDK also records its version and target in `share/malvado/malvado-sdk-info.txt`.
The SDK includes the Lua Language Server type definitions in
`share/malvado/types`. `malvado new` writes a `.luarc.json` file into each
generated Lua project so editors provide autocompletion and type checking for
the public Lua API. See the
[Lua Language Server guide](https://github.com/jepemo/malvado/blob/main/docs/api/lua-language-server.md).


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

The Lua starter contains two files when you create it with the installed SDK.

```text
my-game/
  main.lua
  .luarc.json
```

`main.lua` is the entry script. It returns a game table with an `init`
function that starts the first process. Set `title`, `width`, and `height`
in that table to configure the window. An entry script can also use
`mv.start()`. Use one entry form per run. See the
[declarative game example](https://github.com/jepemo/malvado/tree/main/examples/declarative)
for a complete game table.

`.luarc.json` is editor configuration. The generator writes it when it finds
the installed Lua type definitions. The runtime reads game settings from
your Lua code.

### Add scripts and assets

Add folders as the game grows. For example, put Lua modules in `scripts/`
and place a player image at `assets/sprites/player.png`. These folders are
optional. Load a module with `require("scripts.player")` for a file named
`scripts/player.lua`, and load the image with
`load_asset("assets/sprites/player.png")` when using the starter's global imports.

By default, relative asset paths and local Lua module paths start at the
directory that contains the entry script. This also works when you run
`malvado run my-game/main.lua` from its parent directory.
Use `assets_dir = "assets"` in the returned game table to shorten the
image path to `load_asset("sprites/player.png")`. The runner applies that
setting before `init`. A relative `assets_dir` starts at the entry script's
directory and affects asset lookup only. Keep files inside the project so
they travel with your export. See [game asset paths](https://github.com/jepemo/malvado/blob/main/docs/api/asset-sources.md#game-asset-paths)
for overrides and native games.

### Create a C game

Use `malvado new --lang c my-c-game` to generate `main.c` and
`CMakeLists.txt`. Install CMake and a C compiler. From `my-c-game`,
use `malvado check` to compile and link the starter without running it, or
`malvado run` to build and start it.

## Create a Rust game

Rust's gameplay SDK is experimental. Future 2.x changes can require source
updates. Use `Game` as the public entry
point. Use `Game::windowed` and `game.run()` for a managed window. See the
[support policy](https://github.com/jepemo/malvado/blob/main/docs/api/rust-support.md) and [Game migration](https://github.com/jepemo/malvado/blob/main/docs/development/rust-game-migration.md).
Rust games are native executables; players do not need Cargo or rustc.

Install Cargo, rustc, CMake and a C compiler, then use a matching native SDK
for macOS, Linux or Windows x64 MSVC:

```sh
malvado doctor
malvado new --lang rust my-rust-game
cd my-rust-game
malvado check
malvado build
malvado run
```

Edit `main.rs` to change the game. The starter runs an async process on the native
scheduler. Its named async function draws the greeting and waits for each
new activation. The managed runner handles input, ticks and drawing. Press
Escape or close the window to exit. `Cargo.toml` defines the package, the `game` binary and the dependency
on the SDK's installed `malvado` crate. The CLI supplies the native SDK prefix;
you do not need a Malvado source checkout. If you move the SDK, update the crate
path in the manifest. Rustup automatic toolchain installation is disabled by the
CLI; install the required tools before building.

`check` uses Cargo and can execute dependency build scripts and procedural macros.
It does not start the game. `run` builds first and uses the project directory for
asset paths. Use `Game::new()` for Rust headless hosts. `--headless` is a Lua runner option. See the
[Rust process guide](https://github.com/jepemo/malvado/blob/main/docs/api/rust-processes.md)
for state, frame waits, cancellation and resource lifetimes.

## Export a game

Use `malvado build` from a C or Rust project directory to compile the game.
Use `malvado run` to build and play it. For Lua, run `malvado check` to find
syntax errors, then use `malvado run` to play the scripts.

When you are ready to share the game, run this from its project directory.
The second command starts the exported copy on macOS or Linux.

```sh
malvado export
./dist/game/game
```

On Windows, start it with `.\dist\game\game.exe` in PowerShell.

The exported folder contains the game and its license notices. Copy the full
folder to a matching target computer. Native desktop exports use the current
host SDK. The destination must be new. Use
`malvado export ../another-folder` to choose a different destination.

On macOS and Linux, the destination filesystem must support exclusive
rename. If a shared mount does not support it, export to a local
filesystem and copy the completed folder to the mount.

For C and Rust, export builds the `game` target and includes its executable.
Players do not need the SDK or authoring tools. Source files and build outputs
are excluded. A Rust game's extra dependency runtimes and notices must be
provided by its author; the exporter does not discover them automatically.

For a Lua game, the launcher loads `main.lua` from the exported folder. It supports both a returned game table and `mv.start()`.

On Linux, you can create an archive to share the exported folder:

```sh
tar -czf my-game.tar.gz -C dist game
```

The recipient can extract the archive and run `./game/game`.

On macOS, you can instead create an application bundle with `malvado export --app`. It creates `dist/game.app`. To choose a different name and destination, use a path that ends in `.app`, such as `malvado export --app ../my-game.app`. The name must be valid UTF-8 without control characters. This initial bundle uses a default identifier. It is not signed or notarized by Malvado. Manual clean-machine validation is planned for beta.

To give a game its own macOS identifier, use `malvado export --app --id org.example.my-game`. Replace the example with an identifier for your game. Without `--id`, every app uses `org.malvado.game`.

To share a macOS app, select the `.app` in Finder and choose **Compress**. Send the resulting ZIP file. The recipient can extract it and open the app.

Exported games skip hidden files, and directories named `build` or `dist`. They cannot include symbolic links. For the current limits and project rules, see the [CLI guide](https://github.com/jepemo/malvado/blob/main/docs/development/cli.md).

## Export for Android

Android arm64 platform support is beta.

Use a graphical macOS arm64 or Linux x86_64 host SDK. Install the Android
tools and prepare a separate Android arm64 target SDK. Follow the setup for
[C](https://github.com/jepemo/malvado/blob/main/docs/development/android-c-games.md),
[Lua](https://github.com/jepemo/malvado/blob/main/docs/development/android-runtime.md)
or [Rust](https://github.com/jepemo/malvado/blob/main/docs/development/android-rust-games.md).
Then run this command in the project directory:

```sh
malvado export --target android-arm64 --format apk \
  --runtime-sdk /absolute/path/to/malvado-android-prefix \
  --android-sdk "$ANDROID_SDK_ROOT" --output ../my-game.apk
```

Set `--runtime-sdk` to the installed Malvado Android prefix. Set
`--android-sdk` to the Android tools directory. Install the debug APK on
the supported emulator with `adb install ../my-game.apk`.
For a release APK or AAB, use a Release target SDK and follow the
[release bundle and signing guide](https://github.com/jepemo/malvado/blob/main/docs/development/android-bundles.md).
See the [platform matrix](https://github.com/jepemo/malvado/blob/main/docs/development/platforms.md)
for supported hosts, toolchains and verification limits.
