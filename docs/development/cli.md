# CLI reference

The CLI creates, checks, runs, builds and exports C, Lua and Rust projects.
See the [SDK guide](../site/sdk.md) for installation and the
[platform matrix](platforms.md) for supported targets.

## Command name

Use one user-facing command, `malvado`, for common tasks. For example:

```sh
malvado new my-game
malvado run
malvado run main.lua
```

```text
malvado new [--lang lua|c|rust] PATH
```

Lua is the default. Use `--lang lua` to select it explicitly. Put the language
option before the project path. Invalid arguments return status 2 without
creating a project. If you used `new --c` in an earlier build, use
`new --lang c` now.

For Lua projects, `new` creates a directory and `main.lua`. `run` executes `main.lua` in the current directory, a specified file, or `main.lua` in a specified project directory. When given a directory, `run` enters it before starting the game. When given a Lua file, `run` preserves the caller’s working directory. Both forms use the entry script’s directory for [asset paths](../api/asset-sources.md#game-asset-paths) and local Lua modules. `run --headless [PATH]` is available for scripts that terminate without a window. The `malvado-run` executable also runs Lua scripts.

Managed C and Rust games use the project root for assets. The CLI passes the absolute root to the child through `MALVADO_GAME_DIR`. Desktop launchers pass the packaged content directory, and Android uses its staged content directory. The export includes files inside the project. Absolute paths to external assets work at runtime when accessible. Place assets inside the project for portable exports.

The Lua template returns a game table. The runner also supports `mv.start()`. Exported Lua games load `main.lua` with the same entry rules. See the [Lua process guide](../api/lua-processes.md).

`malvado new --lang c NAME` creates a minimal C game and `CMakeLists.txt`. On macOS, Linux and Windows x64 MSVC, an installed graphical SDK can build it with `malvado build` or build and start it with `malvado run`. Both commands also accept the project directory as an argument. CMake and a C compiler must be available. The client uses `.malvado/build` and rejects an existing build directory that it does not recognize. C projects use a CMake target named `game`. `--headless` is only available for Lua games.

`check` in a C project directory configures, compiles and links the `game`
target through the existing managed C build. As with C builds, an installed
graphical SDK is required. It does not run the game. It can
execute CMake project commands and build scripts, and it produces build
artifacts. A missing tool, invalid C source or link failure returns 1. Pass a
project directory for C; individual-file syntax checking remains a Lua operation.

`check` checks Lua source syntax without executing it. With no argument, it checks `.lua` files recursively from the current directory. Pass a directory to check its Lua files, or a file to check only that file. It skips hidden files, hidden directories, and symbolic links. It reports an error if no Lua files are found. It does not check assets, dependencies, or runtime behavior yet.

`doctor` reports the Lua version, whether CMake, Cargo and rustc are available, whether installed Rust crate sources are present, whether this build has graphics, and whether both export launchers and the required license files are present. These include Malvado, Lua, raylib, and cJSON when asset support is enabled. The CMake check does not test a compiler. Doctor does not open a window or verify the graphics driver or target SDKs. It returns success after the report even when an optional capability is unavailable.

`malvado fpg-export [--transparent-zero] INPUT OUTPUT` converts the supported
8-bit FPG subset into an indexed PNG per sprite and a JSON manifest. It needs
asset support but no window. The destination must not exist. See the
[FPG contract](../api/fpg.md) for transparency, metadata, and cleanup behavior.

Use `malvado --help` to list commands and `malvado --version` to show the client version.

After building with Lua enabled, `cmake --install build --prefix /path/to/sdk` installs `malvado` in the SDK `bin` directory. Add that directory to `PATH` to use the short command.

On macOS, Linux and Windows x64 MSVC, an installed graphical SDK can export a native game folder:

```sh
malvado export
```

Run this command from the project directory. It creates `dist/game`. You can also pass a destination outside the project, for example `malvado export ../my-game-export`. The destination must not exist. The folder contains `game` (`game.exe` on Windows), the project files, and engine dependency licenses. Native C/Rust payloads use `game-bin` (`game-bin.exe` on Windows). Run the player executable to start the game. For Lua, the exporter checks the syntax of all included Lua files. For C, it builds the `game` target and excludes `.c` and `.h` files and `CMakeLists.txt` from the output. Both paths skip hidden entries and directories named `build` or `dist`, and reject symbolic links and special files. The project root cannot contain entries named `game`, `game-bin`, or `licenses`. Windows also reserves the executable names and compares reserved names without case. The exporter prepares the game in a temporary directory and publishes it only after copying succeeds. It does not replace an existing destination. The export targets the current host. It does not cross-compile or validate all dependencies.

On macOS, `malvado export --app [--id ID] [PATH]` creates an application bundle. The default is `dist/game.app`; a specified path must end in `.app`. The file name sets the bundle name and must be valid UTF-8 without control characters. The optional ID must have dot-separated components that contain ASCII letters, digits, or hyphens. Components cannot start or end with a hyphen. Without an ID, the bundle uses `org.malvado.game`, which is shared by all default exports. The bundle contains the launcher, game files, and notices. It is not signed or notarized by Malvado.

If a folder contains both `main.lua` and a C project, `run` and `export` select Lua.

## Android APK export (Lua)

An installed graphical SDK on macOS or Linux can export a Lua project as a
runnable Android arm64 debug APK. The current project must contain `main.lua`.
Both explicit and declarative entry forms use the same player.

```sh
malvado export --target android-arm64 --format apk \
  --runtime-sdk /absolute/path/android-runtime \
  --android-sdk "$ANDROID_SDK_ROOT" \
  --output ../dist/game.apk --id org.example.mygame
```

Build the target runtime once with the [Android runtime commands](android-runtime.md).
Its SDK version must match the host client. The exporter validates Android SDK
metadata and the arm64 ELF library before packaging. It requires Python 3,
Build Tools 34.0.0 and platform 34. Default debug signing also requires
Java/keytool on PATH. The verified baseline
uses the root mise-pinned Temurin JDK. No compiler, Cargo, Gradle or store
credentials are used during export.

The output is one new `.apk` file. Publication is exclusive and does not replace
an existing file. Failed builds remove their owned temporary files. Export skips
hidden entries and folders named `build` or `dist`. Other regular project files,
including nested binary assets and Lua modules, are included. Links and special
files are rejected. `licenses` and `malvado-files.txt` are reserved root names.
Paths cannot contain backslashes, CR/LF/NUL or exceed 1023 UTF-8 bytes. All included
Lua files pass the ordinary CLI syntax check. Dependency notices are packaged.

The optional application ID defaults to `org.malvado.game`. It needs at least two
dot-separated components; each starts with an ASCII letter and then contains
ASCII letters, digits or underscores. The manifest defaults to version code 1,
landscape orientation, minimum API 28 and target API 34. Configure it with
`--version-code` (1–2100000000), `--version-name` and `--title` (1–128 characters,
no controls), `--orientation landscape|portrait|sensor` and `--min-api` (28–34).
Text is XML-escaped. Invalid values fail before tool execution or output-folder
creation. Target API remains pinned to 34. By default, a fresh temporary debug
key signs each debug export. Uninstall an older build before installing another build
with that ID. This is a development APK, not a production signing workflow.

For a release APK, install an `AndroidRuntime` component built with
`-DCMAKE_BUILD_TYPE=Release`, then add `--release`. The exporter validates the
installed runtime profile and sets `android:debuggable="false"`. Without signing
options, it produces an aligned **unsigned** APK. Sign that file before installing
it, or provide an explicit keystore during export:

```sh
malvado export --target android-arm64 --format apk --release \
  --runtime-sdk /absolute/path/release-runtime \
  --android-sdk "$ANDROID_SDK_ROOT" --output ../dist/game-release.apk \
  --id org.example.mygame --version-code 2 --version-name 0.2.0 \
  --keystore /absolute/path/keys/game.p12 --key-alias game \
  --store-password-env GAME_STORE_PASSWORD
```

Set the named password variable in your environment before running the command.
Use `--key-password-env NAME` if the key needs a separate password. Values are
passed to apksigner by environment reference, not as command-line passwords.
Signing options require an existing keystore **outside the project**, a key alias
and a nonempty store-password variable. Both named password variables must be
nonempty when supplied. The exporter does not create, change or
copy that keystore. It verifies the signed APK before exclusive publication.
Debug exports can use the same signing options to keep a stable development key.
The default version name is `release` in release mode and `debug` otherwise.

Every export needs the JDK and D8 to compile the shared Android Activity.
APK generation also needs aapt and zipalign. Explicit APK signing needs
apksigner. Automatic debug-key creation
needs keytool. Signing a file does not publish it to a store. Release verification
uses disposable test credentials and does not establish production key custody.

C projects now use the same command with a full Android target SDK. The C
template contains the required CMake branch. See [C Android games](android-c-games.md).
C export additionally requires CMake and NDK r27c. It supports both debug and
release APKs, source exclusion, the same manifest options and explicit signing.
Rust projects now support the same debug/release APK path with a staticlib
bridge and a target ABI probe runner. New Rust projects include the bridge;
see [Rust Android games](android-rust-games.md). Existing ABI
checks remain mandatory. Release AAB generation is now available through `--format aab --release
--bundle-tools PATH` for C, Lua and Rust. It validates the base module with
bundletool and uses jarsigner for optional signing. See [AAB tools and
commands](android-bundles.md). Other Android export ABIs remain
unavailable. Windows Android authoring is unavailable. The
[alpha.8 acceptance record](../versions/2.0.0-alpha.8.md#accepted-candidate)
covers C, both Lua entry forms and Rust production packages. See the
[platform verification limits](platforms.md#manual-verification-limits)
for physical device coverage.

## Future project tools

The [project tools proposal](../roadmap/project-tools.md) covers project
metadata, additional templates, SDK selection and integrations with the
graphical workshop.

## Rust projects

On macOS, Linux and Windows x64 MSVC, use an installed graphical SDK
that includes the Rust crate sources:

```sh
malvado new --lang rust my-game
cd my-game
malvado check
malvado build
malvado run
```

The template contains `main.rs` and `Cargo.toml`. It uses the safe Rust API,
the native process scheduler and an explicit window/time loop. Escape closes
the window. Cargo owns package metadata; no `malvado.toml` is required. The CLI
supports a binary target named `game`; Cargo defines its source path. Other
binary names and workspaces with multiple packages are outside this initial
contract. The generated dependency points to the installed SDK crate sources.
If you move or replace that SDK, update the dependency path in `Cargo.toml`.
SDK paths with quotation marks, semicolons or control characters are rejected because the native CMake probe does
not support them. Paths with spaces are supported.

A regular `Cargo.toml` identifies a Rust project. A folder with both that file
and a Lua entry or a complete C project is ambiguous and returns 2. The existing
Lua-over-C precedence is unchanged when no Cargo manifest exists. An explicit
Lua script path still runs or checks that script.

`build` invokes `cargo build`; `check` invokes `cargo check`. Both select the
native rustc host target and the `game` binary. They use Debug and a managed
`.malvado/rust` target directory, independent of `CARGO_TARGET_DIR` and ambient
Cargo target configuration. Existing unmanaged directories are rejected.
`run` builds first, then executes the binary with the project as its working
directory and returns the game's exit code. All three accept a project directory
argument, including paths with spaces. `--headless` remains a Lua runner option;
a Rust application defines its own host loop and can use the core without a window.

The CLI supplies its installed SDK prefix to Cargo. CMake, a C compiler, Cargo
and rustc must already be installed. Native ABI/version checks remain in
`malvado-sys`. Missing tools, SDK files, invalid manifests, missing `game` targets
and compiler failures return 1. Rustup automatic toolchain installation is disabled
for CLI child tools; no toolchain installer is invoked. Cargo can download declared
crate dependencies. Rust `check` can execute build scripts and procedural macros;
it does not execute the game binary. `doctor` only reports availability and does
not compile a project or install tools.

`malvado export` builds the Rust `game` binary and uses the same native player
launcher as C exports. The folder contains the executable, assets and engine
notices. Rust source files, Cargo manifests/lockfiles, `target`, hidden entries
and the existing `build`/`dist` exclusions are omitted. Source-named directories
can still contain assets. Symbolic links and special files are rejected before
publication. Existing destinations are preserved. A failed build cannot export
an old binary. Custom source paths in the Cargo binary definition are supported.

On macOS, `export --app` uses the same bundle rules as C/Lua. Exported Rust games
run without Cargo, rustc, a C compiler or the installed SDK. Dependencies added
by the application can require extra runtime libraries and notices; the exporter
does not discover or bundle arbitrary Cargo dependency runtimes.
