# malvado-cli: projects and exporters

Status: proposal for Malvado 2. Requested targets: **Linux, Windows, macOS, and Android**. The CLI and these commands do not exist yet.

## Target experience

```sh
malvado-cli new my-game --template starter
cd my-game
malvado-cli doctor
malvado-cli run
malvado-cli check
malvado-cli export --target linux-x86_64 --output dist/linux
malvado-cli export --target windows-x86_64 --output dist/windows
malvado-cli export --target macos-arm64 --output dist/macos
malvado-cli export --target android-arm64 --format apk --output dist/android
malvado-cli export --target android-arm64 --format aab --release --output dist/android
```

`malvado-cli --version` shows CLI, runtime, project-format, and pinned dependency versions. `export` uses a compatible target runtime and creates a game package with native executable and bundled Lua. It does not automatically convert Lua code to machine code. Initially, scripts are packaged Lua source. Optional bytecode requires control of VM version, architecture, and compatibility, and is not code protection.

The player receives the package and does not need CLI, Lua, LuaRocks, or LÖVE. Authors can export with prebuilt SDK runtimes or build them with documented toolchains.

## Proposed commands

| Command | Purpose | Milestone |
| --- | --- | --- |
| `new NAME --template NAME` | Generate game, configuration, README, and minimal assets | alpha.4 |
| `run [PATH]` | Validate and run in a compatible local runtime | alpha.4 |
| `check [PATH]` | Check Lua syntax, manifest, assets, identifiers, and dependencies | alpha.4 |
| `doctor [--target TARGET]` | Detect SDK, versions, tools, and missing runtimes | alpha.4; Android alpha.6 |
| `export --target TARGET` | Package for target with manifest and checksums | alpha.4/alpha.6 |
| `templates list` | Describe available templates offline | alpha.4 |
| `sdk list` | List installed runtimes/toolchains and compatibility | alpha.4 |
| `sdk install VERSION --target TARGET` | Explicit, verified download of published runtimes | Later, after verified releases |
| `build --target TARGET` | Build native runtime from pinned sources | After SDK packaging |
| `assets import` | Convert validated FPG/Aseprite subset to PNG+JSON library | alpha.5 |
| `assets export --format fpg` | Export a selected FPG variant and document losses | 2.1+, optional |

`check` does not prove that a game works and must not execute arbitrary code to analyze it. `run` does execute the project. Exit codes: 0 success, 1 operation/game failure, 2 invalid usage or configuration. Add structured JSON for CI when the error contract is defined.

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

Window configuration supplies defaults to `mv.start`; explicit options take precedence. Validate entry, relative paths, platform IDs, version, and profiles. Add a lockfile with SDK and tool revisions/hashes when dependency resolution exists. The project schema evolves independently of the runtime version.

Proposed templates: `starter`, `processes`, and `asteroids`; `android-touch` after mobile validation. Starter needs no downloads or external assets. Other templates include only assets with recorded origin and license. Test templates as real games. Avoid copies of examples that later diverge.

## Shared use by malvado-gui

The [GUI](gui.md) uses the same C project, configuration, template, job, validation, and export services. If it initially invokes a CLI subprocess, it needs structured output and a job protocol. It must not parse translated messages. The runner remains a separate process; game diagnostics use snapshots and commands between ticks. The CLI works without a graphics toolkit, and exports do not include ImGui.

## Assets and packaging

See the [format decision](assets.md): PNG+JSON source, FPG/Aseprite adapters, and ZIP container. Proposed commands, not yet available:

```sh
malvado-cli assets import --format fpg --input source.fpg --output assets/imported
malvado-cli assets import --format aseprite-json --input sheet.json --output assets/animation
malvado-cli assets export --format fpg --variant VARIANT --input assets/imported --output dist/graphics.fpg
```

Document the exact variant and limits during implementation. `assets import` does not renumber graphics and reports conversion and losses. FPG `assets export` is later and can reject unrepresentable data. A delivery ZIP is not necessarily mountable by the runtime. Android uses APK assets without an extra nested ZIP by default.

## C implementation

`src/cli` uses C libraries for configuration, validation, packaging, and launch. Include a C template as well as Lua, and declare the project language. C requires the target SDK, headers, library, and toolchain. Lua uses the runner with bundled VM. CI must check each `run` and `export` path. Implementation belongs to alpha.4; Android to alpha.6. See [C architecture](../architecture/c-first.md). Include templates in the binary or SDK with limited variables. Do not use a template engine that executes project code. Select and pin a small C TOML parser with a compatible license. Do not accidentally implement a complete parser.

Launch tools with argument arrays, not shell command strings built from names/paths. Support spaces and Unicode. `new` fails if the destination already contains files. `export` uses staging and atomic result publication without deleting the project. Repeated exports replace only artifacts identified as their own. Build directories, credentials, and `.git` are excluded from packages. Include a file manifest, hashes, licenses, and versions with the artifact.

## Contract by target

| Target | First artifact | Tools and details | Verification |
| --- | --- | --- | --- |
| Linux x86_64 | Folder/archive with executable and assets | Explicit glibc baseline; required libraries/rpath; arm64 later | Clean machine without Lua; input, audio, shutdown |
| Windows x86_64 | Portable ZIP with `.exe`, assets, and required DLLs | Native CI build or validated MinGW; include runtime dependencies | Clean Windows without development tools |
| macOS arm64 | `.app` in ZIP; x86_64 or universal later | Info.plist, icon, resource bundle; release signing/notarization on macOS | Finder launch and self-contained bundle |
| Android arm64 | Debug APK; release APK/AAB | Pinned SDK/NDK/JDK/Gradle, ABI, Activity, assets, native library | Install/run on device or emulator; validate bundle |

Not every target supports every architecture in 2.0. Reject unsupported combinations with a useful diagnostic. AAB is a distribution format and needs specific validation; it is not installed directly like an APK.

Windows verification and effective support are reserved for alpha.6. Alpha.4 accepts macOS and Linux. Desktop support does not imply cross-compilation from every OS. Start with native CI builds per target and versioned SDKs; the exporter selects the runtime. Android needs its toolchain on macOS/Linux/Windows hosts. Publish the host matrix after testing.

## Android: engine work beyond ZIP

Use the [raylib Android build documentation](https://github.com/raysan5/raylib/wiki/Working-for-Android) as the prototype reference. Pin template and toolchain in alpha.6. Do not copy commands from an untested version into the product.

- Integrate a native host with the selected Activity template. Package Lua and C modules as game libraries; no installed interpreter.
- Support touch in logical screen coordinates, DPI, orientation, Back, and on-screen keyboard when applicable. Keyboard-only asteroids does not test Android.
- On background/foreground, pause simulation/audio, restore assets when needed, and reset delta to prevent large jumps.
- APK assets are read-only. Save data in the application's writable directory. Resolve paths through the asset service, not a desktop cwd assumption.
- Test sound, suspension, screen lock, and return on a device.
- Validate `application_id`, `version_code`, icons, orientation, and ABI. Select minSdk/targetSdk when pinning the toolchain, according to current distribution requirements. Do not freeze store values without checking them.
- Use debug APK for iteration. Release export reads keys from a secure path/environment. Do not copy passwords or keystores into the project, logs, template, or package. Signing/notarization require author credentials and are not performed in this documentation phase.

## End-to-end acceptance

Create starter in a path with spaces. Run, package, move the artifact to a clean machine, and play. Repeat with asteroids. Compare manifests/checksums for equivalent builds, normalizing timestamps and order. Do not promise binary identity if signing or tools introduce variation. Android adds installation, touch, pause, audio, and AAB checks. Mark a platform supported only after this flow passes.
