# Get started

Install the Malvado SDK, open the Lua starter, and change the greeting it
draws. The SDK includes the command-line tools and Lua runtime. Use Lua,
C, or experimental Rust to write a game.

## Install the SDK

### macOS and Linux

Use the installer on macOS arm64 (Apple silicon) or Linux x86_64. You need
an internet connection, Bash, curl, tar, and a SHA-256 tool such as
`shasum` or `sha256sum`. The prebuilt SDK runs Lua games without a compiler.

```sh
curl -fsSL https://malvado.dev/install.sh | bash
```

The script downloads the latest stable SDK and verifies its SHA-256
checksum. It installs under `$XDG_DATA_HOME/malvado` when
`XDG_DATA_HOME` is set, or under `~/.local/share/malvado` by default.
Run the `export PATH` line printed by the installer in your current terminal.
Add that line to your shell configuration to use `malvado` in new terminals.

### Windows

On Windows x64, download the SDK archive ending in `windows-AMD64.tar.gz`
and its matching `.sha256` file from
[GitHub Releases](https://github.com/jepemo/malvado/releases).
In PowerShell, open the download directory and compare the archive's
SHA-256 hash from `Get-FileHash` with the hash in the checksum file.
Replace the version in the file name below with the one you downloaded.

```powershell
$sdkArchive = "malvado-sdk-2.1.0-windows-AMD64.tar.gz"
Get-FileHash -Algorithm SHA256 $sdkArchive
Get-Content "$sdkArchive.sha256"
```

When the hashes match, extract the archive into a new, empty directory and
add its `bin` directory to this PowerShell session's PATH.

```powershell
$sdkDir = Join-Path $HOME "malvado-sdk"
if (Test-Path $sdkDir) { throw "Choose a new SDK directory." }
New-Item -ItemType Directory $sdkDir
tar -xzf $sdkArchive -C $sdkDir
$env:Path = "$sdkDir\bin;$env:Path"
```

To keep the command in new terminals, add the SDK's `bin` directory to
your user PATH. The [platform matrix](https://github.com/jepemo/malvado/blob/main/docs/development/platforms.md#delivered-artifacts-and-unavailable-paths)
lists the Windows graphics and Microsoft runtime requirements.

### Verify the installation

```sh
malvado --version
```

The command prints the installed Malvado version. Use a desktop session
with graphics support to open the starter game window.

## Uninstall the SDK

To remove an SDK installed with the script, run:

```sh
curl -fsSL https://malvado.dev/install.sh | bash -s -- --uninstall
```

If you installed with `--prefix`, pass the same `--prefix` when you uninstall.
If you added the SDK to your `PATH`, remove that entry from your shell
configuration.

## Create and run a game

Open a terminal in a directory where you keep projects. Choose a name
that is available there. These commands create and enter `my-game`.

```sh
malvado new my-game
cd my-game
malvado check
malvado run
```

`malvado check` checks the Lua files for syntax errors. `malvado run` runs
`main.lua` and opens a window titled **My game**. Look for
**Hello, Malvado!** near the upper left. Press **Escape** or close the
window to stop the game.

### Change the greeting

Open `main.lua` in a text editor. Find the line with `draw_text` and change
the value of `text` to `"Hello from my game!"`. Save the file and run these
commands again from `my-game`.

```sh
malvado check
malvado run
```

The window should display **Hello from my game!**. Try changing `x` from
`40` to `120` to move the greeting to the right.

### Read the starter

The Lua runner opens `main.lua`. The script returns a table with the window
title and an `init` function. When the game starts, `init` starts the greeting
process. Each time it runs, that process draws the text and calls `frame()`
to wait for its next turn. A larger game can give the player and each enemy
their own process in the same way.

## Lua in VS Code

Open the project folder in VS Code and install the
[Lua extension](https://marketplace.visualstudio.com/items?itemName=sumneko.lua)
(`sumneko.lua`). Open `main.lua` and hover over `draw_text` to see its
signature. The `.luarc.json` created by `malvado new` tells Lua Language
Server where to find the installed type definitions.

Use the same project settings in any editor that runs Lua Language Server.
If you move the SDK, update the paths in `workspace.library` in
`.luarc.json`. For an existing project without that file, follow the
[manual setup guide](https://github.com/jepemo/malvado/blob/main/docs/api/lua-language-server.md#manual-setup-for-existing-projects).

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
