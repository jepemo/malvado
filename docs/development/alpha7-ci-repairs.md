# Alpha.7 CI verification repairs

The acceptance audit inspected CI for `2705748cd2adf6bbfa881d9bc124a5bb526c2adb`.
The [CI run](https://github.com/jepemo/malvado/actions/runs/43238681414) has a
Linux graphics dependency-download failure. The earlier
[renderer CI run](https://github.com/jepemo/malvado/actions/runs/43234952762)
records Miri failures on macOS and Linux. These failures prevent an unqualified
A7.6 acceptance claim.

## Miri and native coverage

The new native text unit tests called `mv_abi_version` under Miri. This fails
before text operations because Miri does not execute C FFI. Move both tests to
`bindings/rust/malvado/tests/text_values.rs`, with the same public-API assertions.
The library target remains the pure Rust ownership proof. ABI checks stay active
in native tests and production calls.

Local evidence: seven Miri library tests pass. Native text integration tests pass
in Debug and Release through the relocated SDK, with Clippy/format checks and
ASan/UBSan native integration. The repair separates the native and interpreter verification targets.

## Lua archive failover

The Linux graphics job failed when the Lua origin returned HTTP 500. Keep Lua
5.4.8 and its SHA-256 unchanged. Supply the `www.lua.org` and `lua.org` official
host names as ordered FetchContent URLs. CMake uses the next URL when an origin
fails. Every accepted archive must match the original version and SHA-256.

`lua_download_retry` exercises the actual declaration with a local HTTP server.
A first HTTP 500 followed by a valid archive succeeds. Two failing endpoints fail
configuration. A valid archive with the wrong content/hash is rejected before
extraction. The fixture uses no external network and overrides only the test
URLs and payload hash; it also checks the production release hash stays pinned.
CMake can perform its own additional hash-retry cycles.

The live alternate-host download probe was reset/timed out in this environment.
Local fault injection verifies failover and integrity; fresh remote CI must still
verify real download availability and the complete host matrix. A7.6, A7.9 and version acceptance remain open.


## Installed SDK authoring order

The [CLI implementation run](https://github.com/jepemo/malvado/actions/runs/43388888367)
for `5171fdf74a915a8701da9259d5d4d0e921d1d88a` failed both fresh SDK consumer
jobs. `sdk_clean.cmake` had already removed the SDK before the workflow started
the Rust CLI checks. Move those checks into `sdk_clean.cmake`, before removal,
behind explicit `CHECK_RUST=ON`. C/Lua-only callers keep their toolchain boundary;
Rust acceptance requires Cargo and Python instead of silently skipping them.

The candidate collection job also requires the Rust SDK job. It must not collect
an accepted candidate when the Rust ownership, Miri or sanitizer checks fail.

Local macOS arm64 evidence: package the SDK, extract it outside the checkout,
run `sdk_clean.cmake` with `CHECK_RUST=ON` and `VISUAL=ON`, complete Rust authoring
and native folder/app player checks, remove the SDK, then complete the existing
C/Lua player checks. The SDK is absent at completion and the Rust relocated
player remains available. Fresh host CI and the full A7.9 gate remain open.


## CLI static analysis configuration

The same implementation run completed the sanitizer tests, then failed while
compiling `src/build.c` for static analysis: the standalone command did not define
`MALVADO_RUST_CRATE_REL`. Supply the standard installed crate path used by the
workflow's CMake configuration. Keep static analysis enabled and preserve the
existing source list and diagnostics.

The repaired command passes with Clang on macOS arm64 and Ubuntu 24.04 arm64.
This verifies the CLI translation unit and its production export branch; it does
not claim the other analyzer diagnostics or the entire CI job are resolved.


## Paced silent audio tests

The Linux visual job in the same run failed `visual_audio_resource`: a paused
sound had already lost its playback root. Ubuntu 24.04 arm64 with Xvfb and the
same ALSA null PCM reproduced the failure. That PCM consumes frames without
wall-clock pacing; the short sound can finish before Lua pauses it. This is not
a valid clock for pause, completion and stream-maintenance acceptance.

`MALVADO_TEST_NULL_AUDIO=ON` selects only miniaudio's paced null device through
its existing compile definitions. The visual, Rust graphics and graphics
sanitizer tasks select it explicitly. The option defaults to OFF; normal SDKs
and packages retain their platform audio backend. No third-party source, native
playback semantics, Lua roots or fixture assertions change.

The unchanged native/Lua audio fixtures pass five consecutive runs on Ubuntu
24.04 arm64 and report `miniaudio | Null`. The full Linux Xvfb visual suite passes
83 tests, including keyboard/mouse/text injection. macOS arm64 passes 68 visual
tests and the Rust native graphical ASan/UBSan task with the paced backend.
The normal graphical SDK package and author/player checks use the default
platform configuration. These tests do not certify audible output or physical
devices; those remain beta checks. Final remote CI remains A7.9 work.


## Cargo-absent compiler proxy isolation

The [integration acceptance run](https://github.com/jepemo/malvado/actions/runs/43392182877)
for `c7ec2010b03571cbdfa68d8701882c4ebf328b6a` passed every required job except
the Linux fresh SDK consumer. The Cargo-absent fixture linked `rustc` into an
otherwise empty PATH. A mise/env-based compiler proxy can need its own tools on
PATH, so the compiler query failed before the intended Cargo rejection.

Use a fixture wrapper that restores the original PATH only for the compiler
child. The CLI still sees no Cargo. Verify the compiler query succeeds in that
restricted environment before asserting the Cargo error. No product command,
SDK discovery or missing-tool behavior changes.

Local evidence uses an env-based rustc proxy that cannot run on the old limited
PATH. The full relocated SDK CLI suite then passes, including the bounded
starter/player, missing Cargo, compiler recovery, exports and independent player
checks. Final fresh-runner acceptance remains required.
