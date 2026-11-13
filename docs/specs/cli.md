# Malvado client: projects and exporters

Status: in progress for Malvado 2. Requested targets: **Linux, Windows, macOS, and Android**. Initial `malvado new PATH`, `malvado run [PATH]`, `malvado check [PATH]`, `malvado doctor`, and local `malvado export [PATH]` commands exist. Project metadata and the other commands below remain proposals.

## Command name

Use one user-facing command, `malvado`, for common tasks. For example:

```sh
malvado new my-game
malvado run
malvado run main.lua
malvado gui
```

`new --lang lua` is equivalent to `new`; `new --lang c` is equivalent to `new --c`. Unknown languages and conflicting options return 2 without creating a project.

`new` creates a directory and `main.lua`. `run` executes `main.lua` in the current directory, a specified file, or `main.lua` in a specified project directory. When given a directory, `run` uses it as the working directory, so modules and assets resolve relative to the project. When given a file, paths remain relative to the current working directory. `run --headless [PATH]` is available for scripts that terminate without a window. The existing `malvado-run` remains available. A separate `gui` command is a proposal for the optional editor. The editor can still run the game in a separate process. Do not use `run --gui` until its meaning is clear.

The Lua template returns a game table. The runner also supports `mv.start()`. Exported Lua games load `main.lua` with the same entry rules. See the [Lua process guide](../api/lua-processes.md).

`malvado new --c NAME` creates a minimal C game and `CMakeLists.txt`. On macOS and Linux, an installed graphical SDK can build it with `malvado build` or build and start it with `malvado run`. Both commands also accept the project directory as an argument. CMake and a C compiler must be available. The client uses `.malvado/build` and rejects an existing build directory that it does not recognize. C projects use a CMake target named `game`. `--headless` is only available for Lua games.

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
[FPG contract](fpg-8.md) for transparency, metadata, and cleanup behavior.

Use `malvado --help` to list commands and `malvado --version` to show the client version.

After building with Lua enabled, `cmake --install build --prefix /path/to/sdk` installs `malvado` in the SDK `bin` directory. Add that directory to `PATH` to use the short command. Full desktop packages and clean-machine validation remain pending.

On macOS and Linux, an installed graphical SDK built with static libraries can make an initial local game folder:

```sh
malvado export
```

Run this command from the project directory. It creates `dist/game`. You can also pass a destination outside the project, for example `malvado export ../my-game-export`. The destination must not exist. The folder contains `game`, the project files, and engine dependency licenses. Run `game` to start the game. For Lua, the exporter checks the syntax of all included Lua files. For C, it builds the `game` target and excludes `.c` and `.h` files and `CMakeLists.txt` from the output. Both paths skip hidden entries and directories named `build` or `dist`, and reject symbolic links and special files. The project root cannot contain entries named `game`, `game-bin`, or `licenses`. The exporter prepares the game in a temporary directory and publishes it only after copying succeeds. It does not replace an existing destination. The export targets the current host and still needs clean-machine validation. It does not cross-compile or validate all dependencies.

On macOS, `malvado export --app [--id ID] [PATH]` creates an initial application bundle. The default is `dist/game.app`; a specified path must end in `.app`. The file name sets the bundle name and must be valid UTF-8 without control characters. The optional ID must have dot-separated components that contain ASCII letters, digits, or hyphens. Components cannot start or end with a hyphen. Without an ID, the bundle uses `org.malvado.game`, which is shared by all default exports. The bundle contains the launcher, game files, and notices. It is not signed or notarized by Malvado. Clean-machine verification remains pending.

If a folder contains both `main.lua` and a C project, `run` and `export` select Lua. C project builds and exports are initially available on macOS and Linux with the installed graphical SDK.

Define the remaining paths, project discovery, errors, packaging, and the relationship between the client, runner, and optional GUI before implementation. A single command for the user does not require one executable for every component. The commands and options in the target experience below are proposals unless described above.

## Target experience

```sh
malvado new my-game --template starter
cd my-game
malvado doctor
malvado run
malvado check
malvado export --target linux-x86_64 --output dist/linux
malvado export --target windows-x86_64 --output dist/windows
malvado export --target macos-arm64 --output dist/macos
malvado export --target android-arm64 --format apk --output dist/android
malvado export --target android-arm64 --format aab --release --output dist/android
```

`malvado --version` shows CLI, runtime, project-format, and pinned dependency versions. `export` uses a compatible target runtime and creates a game package with native executable and bundled Lua. It does not automatically convert Lua code to machine code. Initially, scripts are packaged Lua source. Optional bytecode requires control of VM version, architecture, and compatibility, and is not code protection.

The player receives the package and does not need CLI, Lua, LuaRocks, or LÖVE. Authors can export with prebuilt SDK runtimes or build them with documented toolchains.

## Proposed commands

| Command | Purpose | Milestone |
| --- | --- | --- |
| `new NAME --template NAME` | Generate game, configuration, README, and minimal assets | alpha.4 |
| `run [PATH]` | Validate and run in a compatible local runtime | alpha.4 |
| `check [PATH]` | Check Lua syntax, manifest, assets, identifiers, and dependencies | alpha.4 |
| `doctor [--target TARGET]` | Detect SDK, versions, tools, and missing runtimes | alpha.4; Android alpha.8 |
| `export --target TARGET` | Package for target with manifest and checksums | alpha.4/alpha.8 |
| `templates list` | Describe available templates offline | alpha.4 |
| `sdk list` | List installed runtimes/toolchains and compatibility | alpha.4 |
| `sdk install VERSION --target TARGET` | Explicit, verified download of published runtimes | Later, after verified releases |
| `build --target TARGET` | Build native runtime from pinned sources | After SDK packaging |
| `assets import` | Convert validated FPG/Aseprite subset to PNG+JSON library | alpha.6 |
| `assets export --format fpg` | Export a selected FPG variant and document losses | Future proposal, unassigned |

`check` does not prove that a game works. Lua syntax checks do not execute scripts. C checks execute the native build, without starting the game. Rust checks use Cargo, which can execute project and dependency build scripts and procedural macros. `run` does execute the project. Exit codes: 0 success, 1 operation/game failure, 2 invalid usage or configuration. Add structured JSON for CI when the error contract is defined.

## Generated project

```text
my-game/
  malvado.toml
  main.lua
  assets/
  README.md
  .gitignore
```

Example proposed configuration; no parser exists yet:

```toml
schema = 1

[project]
name = "My game"
id = "org.example.mygame"
version = "0.1.0"
entry = "main.lua"

[runtime]
version = "2.0.0-alpha.4"
api_profile = "malvado-2"

[window]
width = 800
height = 600
title = "My game"

[assets]
include = ["assets/**", "*.lua"]

[android]
application_id = "org.example.mygame"
version_code = 1
orientation = "landscape"
```

Window configuration supplies defaults to either Lua entry form; script options take precedence. Validate entry, relative paths, platform IDs, version, and profiles. Add a lockfile with SDK and tool revisions/hashes when dependency resolution exists. The project schema evolves independently of the runtime version.

Proposed templates: `starter`, `processes`, and `asteroids`; `android-touch` after mobile validation. Starter needs no downloads or external assets. Other templates include only assets with recorded origin and license. Test templates as real games. Avoid copies of examples that later diverge.

## Shared use by malvado-gui

The future [GUI](gui.md) should reuse project and export behavior. The current exporter has an internal interface, but a complete stable service API does not yet exist. Design that interface during GUI work. If it initially invokes a CLI subprocess, it needs structured output and a job protocol. It must not parse translated messages. The runner remains a separate process; game diagnostics use snapshots and commands between ticks. The CLI works without a graphics toolkit, and exports do not include ImGui.

## Assets and packaging

See the [format decision](assets.md): PNG+JSON source, FPG/Aseprite adapters, and ZIP container. Proposed commands, not yet available:

```sh
malvado assets import --format fpg --input source.fpg --output assets/imported
malvado assets import --format aseprite-json --input sheet.json --output assets/animation
malvado assets export --format fpg --variant VARIANT --input assets/imported --output dist/graphics.fpg
```

Document the exact variant and limits during implementation. `assets import` does not renumber graphics and reports conversion and losses. FPG `assets export` is later and can reject unrepresentable data. A delivery ZIP is not necessarily mountable by the runtime. Android uses APK assets without an extra nested ZIP by default.

## C implementation

`src/cli` uses C libraries for configuration, validation, packaging, and launch. Include a C template as well as Lua, and declare the project language. C requires the target SDK, headers, library, and toolchain. Lua uses the runner with bundled VM. CI must check each `run` and `export` path. Implementation belongs to alpha.4; Android to alpha.8. See [C architecture](../architecture/c-first.md). Include templates in the binary or SDK with limited variables. Do not use a template engine that executes project code. Select and pin a small C TOML parser with a compatible license. Do not accidentally implement a complete parser.

Launch tools with argument arrays, not shell command strings built from names/paths. Support spaces and Unicode. `new` fails if the destination already contains files. `export` uses staging and atomic result publication without deleting the project. Repeated exports replace only artifacts identified as their own. Build directories, credentials, and `.git` are excluded from packages. Include a file manifest, hashes, licenses, and versions with the artifact.

## Contract by target

| Target | First artifact | Tools and details | Verification |
| --- | --- | --- | --- |
| Linux x86_64 | Folder/archive with executable and assets | Explicit glibc baseline; required libraries/rpath; arm64 later | Clean machine without Lua; input, audio, shutdown |
| Windows x86_64 | Portable ZIP with `.exe`, assets, and required DLLs | Native CI build or validated MinGW; include runtime dependencies | Clean Windows without development tools |
| macOS arm64 | `.app` in ZIP; x86_64 or universal later | Info.plist, icon, resource bundle; release signing/notarization on macOS | Finder launch and self-contained bundle |
| Android arm64 | Debug APK; release APK/AAB | Pinned SDK/NDK/JDK/Gradle, ABI, Activity, assets, native library | Install/run on device or emulator; validate bundle |

Not every target supports every architecture in 2.0. Reject unsupported combinations with a useful diagnostic. AAB is a distribution format and needs specific validation; it is not installed directly like an APK.

Windows verification and effective support are reserved for alpha.8. Alpha.4 accepts macOS and Linux. Desktop support does not imply cross-compilation from every OS. Start with native CI builds per target and versioned SDKs; the exporter selects the runtime. Android needs its toolchain on macOS/Linux/Windows hosts. Publish the host matrix after testing.

## Android: engine work beyond ZIP

Use the [raylib Android build documentation](https://github.com/raysan5/raylib/wiki/Working-for-Android) as the prototype reference. Pin template and toolchain in alpha.8. Do not copy commands from an untested version into the product.

- Integrate a native host with the selected Activity template. Package Lua and C modules as game libraries; no installed interpreter.
- Support touch in logical screen coordinates, DPI, orientation, Back, and on-screen keyboard when applicable. Keyboard-only asteroids does not test Android.
- On background/foreground, pause simulation/audio, restore assets when needed, and reset delta to prevent large jumps.
- APK assets are read-only. Save data in the application's writable directory. Resolve paths through the asset service, not a desktop cwd assumption.
- Test sound, suspension, screen lock, and return on a device.
- Validate `application_id`, `version_code`, icons, orientation, and ABI. Select minSdk/targetSdk when pinning the toolchain, according to current distribution requirements. Do not freeze store values without checking them.
- Use debug APK for iteration. Release export reads keys from a secure path/environment. Do not copy passwords or keystores into the project, logs, template, or package. Signing/notarization require author credentials and are not performed in this documentation phase.

## End-to-end acceptance

Create starter in a path with spaces. Run, package, move the artifact to a clean machine, and play. Repeat with asteroids. Compare manifests/checksums for equivalent builds, normalizing timestamps and order. Do not promise binary identity if signing or tools introduce variation. Android adds installation, touch, pause, audio, and AAB checks. Mark a platform supported only after this flow passes.

## Rust projects

On macOS and Linux, use an installed graphical SDK that includes the Rust crate
sources:

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
does not discover or bundle arbitrary Cargo dependency runtimes. This is a native
host export, with Windows/Android and cross-compilation still in alpha.8. See the
[Rust migration plan](../development/rust-migration.md) for the broader scope.
