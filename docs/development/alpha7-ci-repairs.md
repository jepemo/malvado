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

