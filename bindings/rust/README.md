# Rust native SDK

This workspace currently provides `malvado-sys`, the unsafe native boundary.
It is not yet a safe game API. Async processes and graphics wrappers remain
in the [alpha.7 checklist](../../docs/versions/2.0.0-alpha.7.md).

Use the project toolchain and run all local SDK checks:

```sh
mise install
mise run test-rust
```

`mise` pins Rust, CMake and Python, including the required Clippy and rustfmt components. The task builds C-only Debug and Release
SDKs without Cargo on their native build PATH, installs and relocates each SDK
to a path with spaces, then runs Rust tests, clippy and formatting. It also
checks missing, incomplete, incompatible, and dynamic-only SDKs and builds an
independent offline consumer from the installed crate sources. No native source-tree
paths are needed by that consumer. The same task runs in macOS/Linux CI.

For an independent Cargo project, depend on `<prefix>/share/malvado/rust/malvado-sys`
by path and set `MALVADO_SDK_PREFIX` to your installed native SDK:

```sh
MALVADO_SDK_PREFIX="/path/to/installed sdk" cargo test
```

The crate currently declares context, cadence, process identity/lifecycle and
resource operations. See the installed `malvado/malvado.h` for pointer,
ownership, errors and thread rules. Raw calls are unsafe. Call `check_abi()`
before other native operations. Return values are C integers, not Rust enums;
unknown values must not be transmuted into Rust enum variants. Callback panics
must never unwind across C. `malvado-sys` does not catch them for its callers.

## Native discovery contract

The prefix is required. Cargo does not search the checkout, build the engine,
download sources, install tools, or fall back to a system library. `build.rs`
uses CMake to find only the installed package in the prefix. A small C probe
checks every declared native signature against its header and reports field
sizes, alignments, offsets, and enum values. Rust checks these at compile time
for every consumer. Negative fixtures verify signature and layout mismatch rejection. It runs the ABI
query first; a different revision stops validation before the release query.
An exact release match is also required. New incompatible libraries must
increment the ABI revision before changing any other query contract.

The supported product is the existing `libmalvado_core.a` archive with its
math-library dependency. Dynamic-only SDKs are explicitly rejected: Malvado
does not currently build a shared core product. There is no dynamic-loader
path to configure for the static core. A future graphics wrapper must add and
test its backend dependencies rather than assume this core-only link suffices.
Cargo's `links` key prevents duplicate sys packages for the same native engine.

The host C compiler must match the Rust target. The executable probe detects
wrong native architectures through link or execution errors. Cross compilation,
Windows and Android are not supported by this initial SDK. A CMake package
which resolves the core archive outside the selected prefix is rejected.
SDK prefixes cannot contain newlines, carriage returns or semicolons.

No crates.io package, graphics wrapper, player export, or completed alpha.7
release is implied. The development engine and crate identifiers now read
`2.0.0-alpha.7`; the milestone is still in progress.
