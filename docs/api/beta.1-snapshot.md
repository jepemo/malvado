# Beta.1 API snapshot

The gameplay API baseline is the accepted [alpha.8 revision `2a20b7a`](../versions/2.0.0-alpha.8.md#accepted-candidate).
The runtime, headers, bindings and examples at the start of beta.1 match
that revision. Review later API changes against this snapshot. The
[development-version policy](compatibility.md) permits approved 2.x breaking
changes with updated contracts, tests, migration notes and ABI/SDK checks.

## Public interfaces

| Language | Inventory and source | Build requirements |
| --- | --- | --- |
| C | [Generated function inventory and public headers](current-c-api.md), [public type and constant map](process-inventory.md), and [C lifecycle contract](c-contract.md) | Link `Malvado::core` for scheduling and core data. Add `Malvado::assets` for asset metadata and `Malvado::graphics` for the desktop backend. |
| Lua | [Flat exports and constants](lua-globals.md), [namespaces and object methods](lua-namespaces.md), and [process fields and runner](lua-processes.md) | Use the bundled runner. Asset namespaces depend on the build. Audio and graphics resource creation require the applicable platform services. |
| Rust (experimental) | [Safe crate exports](../../bindings/rust/malvado/src/lib.rs) and [raw boundary](../../bindings/rust/malvado-sys/src/lib.rs). Cargo generates the public type, trait and method reference from both installed crates. | The default safe crate uses the scheduler and core types. Enable `assets` or `graphics` with the matching native SDK components. Android requires a target SDK and executable ABI probes. |

## Behavior and limits by family

| Family | Contract and boundaries |
| --- | --- |
| Scheduler, lifecycle, time and messages | [Process map](process-inventory.md), [runner timing](runner-timing.md), [C contract](c-contract.md), [Lua processes](lua-processes.md), [Rust processes](rust-processes.md). C and Rust hosts drive ticks. The Lua runner drives ticks and rendering. Handles stay on their owner thread and do not keep a process alive. Cancellation runs at native safe points. |
| Window, rendering and fades | [Platform map](platform-inventory.md), [drawing](drawing.md), [draw state](draw-state.md), [fades](fades.md), [Rust window](rust-window.md), [Rust drawing](rust-drawing.md). C and Rust draw explicitly. The Lua runner prepares descriptors and invokes the native process pass. Headless mode has no real window. |
| Assets, images and animation | [Asset map](asset-inventory.md), [images](images.md), [animation](animations.md), [FPG](../specs/fpg-8.md), [Rust atlas](rust-atlas.md), [Rust FPG](rust-fpg.md). PNG textures need a graphics service. Asset metadata can run headless. Aseprite support reads exported JSON. FPG support covers the documented subset. |
| Cameras and collision | [Cameras](cameras.md), [collision map](collision-inventory.md), [Rust cameras and animation](rust-camera-animation.md), [Rust collision](rust-collision.md). Worlds contain static rectangles and process colliders use the selected geometric shape. |
| Text, keyboard and controllers | [Text](text.md), [input map](input-inventory.md), [keyboard](keyboard.md), [Rust fonts](rust-font.md), [Rust input](rust-input.md). The packaged Android Activity supports committed IME text. Input snapshots update on the window thread. |
| Audio | [Audio map](audio-inventory.md), [audio maintenance](audio-maintenance.md), [Rust audio](rust-audio.md). Sounds use WAV and streams use WAV or OGG. C and Rust hosts service active streams. The Lua runner services them automatically. |

See the [language matrix](language-parity.md) for differences and the
[family review](family-review.md) for excluded or exploratory capabilities.
The [platform matrix](../development/alpha8-platforms.md) records native macOS,
Linux and Windows authoring, macOS app bundles and Android arm64 APK/AAB
exports from macOS and Linux. Android x86_64 fixtures are emulator evidence.
Physical devices and audible output have separate beta.1 gates.

## Ownership and errors

- C callers check the documented status, pointer or integer result for each
  operation. A successful managed adoption transfers the native object.
  Failed adoption leaves it with the caller. Retains need matching releases.
  End process work before destroying its context. Release direct platform
  objects before closing their window or audio device.
- Lua reports invalid arguments and failed operations through the documented
  error or sentinel for each call. Userdata and queued work hold native
  references. End a process to release its owner reference. Lua references
  can extend the resource lifetime, and collection timing is unspecified.
- Rust callers use typed `Result` and `Error` values. Dropping `Game`
  cancels remaining processes. Dropping a retained wrapper releases its
  reference. Safe platform wrappers keep their required services alive.
  `malvado-sys` callers follow the C pointer, thread and callback rules.

See [resource lifetime](resource-lifetime.md), [native adoption](managed-platform-resources.md),
and [process cleanup](process-cleanup.md) for detailed rules.

## Reproduce the inventory

From a checkout with a standard build:

```sh
ctest --test-dir build --output-on-failure --no-tests=error \
  -R '^(c_api_inventory|.*_api_map(_test)?|public_enum_map(_test)?|family_review(_test)?|lua_(global|namespace|module)_inventory)$'
```

Generate Rust references from a full installed SDK with the project Rust
toolchain. Set `MALVADO_SDK_PREFIX` to its absolute installation path.

```sh
RUSTDOCFLAGS="-D warnings" cargo doc --offline --no-deps \
  --workspace --all-features --manifest-path "$MALVADO_SDK_PREFIX/share/malvado/rust/Cargo.toml" \
  --target-dir build/rustdoc
```

Export `MALVADO_SDK_PREFIX` before running this command. Open
`build/rustdoc/doc/malvado/index.html` for gameplay or
`build/rustdoc/doc/malvado_sys/index.html` for the raw boundary.
