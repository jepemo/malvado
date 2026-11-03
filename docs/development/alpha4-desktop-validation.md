# Alpha.4 desktop validation

Use this procedure for manual desktop testing in beta. It does not block alpha.4. Record the source commit, SDK archive name, host operating system and version, architecture, and results in the [beta.1 checklist](../versions/2.0.0-beta.1.md).

Use one macOS computer and one Linux computer. Each SDK must match its host. For the final game check, use a second computer with the same operating system and architecture. That computer must not have the Malvado SDK installed. Both computers need a working graphics session.

## Get the candidate

Open a successful CI run for the commit to test. Download its `sdk-candidate-` artifact from GitHub Actions. It contains a macOS archive, a Linux archive, and a checksum for each archive. These are development candidates, not release packages.

Extract the GitHub artifact. In the directory that contains the SDK archives, check the archive for this computer. On macOS, run:

```sh
shasum -a 256 -c malvado-sdk-*-darwin-*.tar.gz.sha256
```

On Linux, run:

```sh
sha256sum -c malvado-sdk-*-linux-*.tar.gz.sha256
```

The command must report `OK`. Create an empty directory for the SDK. Extract only the matching archive into that directory, then add its `bin` directory to `PATH` in this terminal. See [SDK and projects](../site/sdk.md) for an example. Run `malvado --version` and `malvado doctor`. Record the output. C projects also need CMake and a C compiler.

## Create and export games

In an empty working directory, create a Lua game:

```sh
malvado new lua-game
cd lua-game
malvado check
malvado run
malvado export
./dist/game/game
cd ..
```

Each run must open the game window. Close it before the next command. The export must contain the game files and a `licenses` directory.

Create a C game:

```sh
malvado new --c c-game
cd c-game
malvado build
malvado run
malvado export
./dist/game/game
cd ..
```

Each run must open the game window. Close it before the next command. The export must contain `game`, `game-bin`, and a `licenses` directory.

On macOS, create an application bundle in each project with `malvado export --app`. Open each `dist/game.app` in Finder and close the window. Keep the two bundles in separate directories.

## Test on the second computer

Copy each complete `dist/game` directory to the second computer. Keep the C and Lua games in separate directories. For each game, open a terminal in its directory and run:

```sh
./game
```

Each game must open and close correctly without the SDK or an installed Lua interpreter. Record failures, including missing system libraries and graphics errors. Test input and audio with a game that uses them; the generated starter does not cover those features.

On macOS, also copy the complete application bundles to the second computer. Open each bundle in Finder and check that the window opens and closes. The initial bundles are not signed or notarized by Malvado. Record any macOS security messages.

This procedure does not establish a minimum operating system version. It does not verify signing, notarization, or cross-platform exports.

## Optional Linux container check

With Docker running, test a Linux x86-64 SDK archive and its adjacent checksum file:

```sh
sh tools/verify-linux-candidate.sh /path/to/malvado-sdk-2.0.0-alpha.4-dev-linux-x86_64.tar.gz
```

The script downloads Ubuntu images and packages. It builds and exports games in one container, then runs the exports in another without the SDK or development tools. It uses a virtual display and software rendering. On another architecture, Docker must support x86-64 emulation. This check does not replace a test on native hardware.
