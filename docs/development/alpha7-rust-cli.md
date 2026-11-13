# Alpha.7 Rust CLI acceptance

A7.7a and A7.7b implement the Rust author workflow and native player export.
The [CLI contract](../specs/cli.md#rust-projects) defines detection, Cargo metadata,
SDK selection, commands, errors and limits. A7.6 [family integration](../versions/2.0.0-alpha.7-integration.md) and the
A7.7 CLI implementation are accepted. A7.9 host CI and candidate packages are accepted in the
[final evidence](../versions/2.0.0-alpha.7-acceptance.md).

## Automated checks

Run `mise run test-rust-cli` after installing the pinned development tools.
This task builds the standard SDK, installs it outside the checkout, moves the
installation and tests a separate project. It does not install a toolchain.
For a display host, also run:

```sh
mise exec -- python3 tools/test_rust_cli.py --visual
```

`tests/rust_cli.py` also accepts an extracted SDK's `--cli` path and an unused
`--work` directory. The fresh SDK consumer jobs run it on macOS and Linux.
Linux uses the existing Xvfb display for the additional `--visual` checks.

The suite covers generated manifest/source compilation, project preservation,
explicit language selection, ambiguous detection, native-host target selection,
managed output, incremental builds, missing tools/SDK/compiler, invalid
manifest/target/source, a two-tick scheduler trace and game exit-code propagation.
It checks compiler-error recovery without deleting the project's build directory.
The sys build script resets only its generated native probe directory when Cargo
reruns it, so failed compiler settings cannot poison later SDK validation.

Export checks use a custom Cargo source path, preserve assets and notices,
exclude sources and build output, preserve existing destinations and reject
symbolic links, reserved names and stale executable export after build failure.
The relocated folder player runs with an empty PATH and the SDK hidden. The
suite inspects its runtime library dependencies. On macOS it also runs the app
bundle. Display checks bound the actual generated host loop and run both its
author and exported graphical executables.

## Local evidence and limits

macOS arm64 passed the CLI suite with `--visual`, standard tests (199), headless
tests (199), C-only tests (72), visual tests (68), and `mise run test-rust` including Debug/Release,
Clippy, format, rejected SDKs and relocated consumers. The separate graphics-only
workspace targets compile in Debug and Release; this combination is now part of
`test-rust-graphics` to prevent optional assets from masking graphics errors.

The complete packaged-SDK flow also passes on Ubuntu 24.04 arm64 with Xvfb.
A Release SDK is packaged and extracted outside the source tree; Rust authoring
runs before the deliberate SDK removal, and the C/Lua players then run without
that SDK. Rust checks include the graphical starter/player and a Cargo-defined
custom source path. The Linux test SDK uses the paced silent audio configuration;
the macOS package check uses the normal platform backend.

The [accepted CI revision](../versions/2.0.0-alpha.7-acceptance.md) passes both
fresh SDK consumer jobs. This evidence does not
claim Windows/Android, cross-compilation, arbitrary Cargo workspace selection or
automatic packaging of third-party game dependency runtimes. No tag or release
is created by these checks.
