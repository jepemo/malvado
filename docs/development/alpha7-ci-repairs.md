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
