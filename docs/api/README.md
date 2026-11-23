# Engine API reference

C, Lua and experimental Rust use the same process scheduler. Start with the
[gameplay guide](../site/api.md) for examples or the [language matrix](language-parity.md)
for differences between bindings.

## Language references

| Language | Entry points and exact exports |
| --- | --- |
| C | [Process and context contract](c-contract.md) and [generated function inventory](current-c-api.md). |
| Lua | [Processes and runner](lua-processes.md), [global functions and constants](lua-globals.md), and [namespace functions](lua-namespaces.md). |
| Rust | [Processes](rust-processes.md), [SDK setup](../../bindings/rust/README.md), and [support and ABI policy](rust-support.md). |

## Gameplay contracts

- **Timing and ownership:** [runner timing](runner-timing.md), [cleanup](process-cleanup.md),
  [resource lifetime](resource-lifetime.md) and [native resource adoption](managed-platform-resources.md).
- **Graphics:** [drawing](drawing.md), [graphics geometry](graphic-geometry.md),
  [visual order](process-visual-order.md), [cameras](cameras.md) and [fades](fades.md).
- **Assets:** [images and atlases](images.md), [animation](animations.md),
  [asset sources](asset-sources.md) and [FPG import](fpg.md).
- **Game services:** [collision and worlds](collision.md), [text](text.md),
  [input](keyboard.md) and [audio](audio.md).

## Verification and reference research

The engine tests check the [process](process-inventory.md), [platform](platform-inventory.md),
[asset](asset-inventory.md), [collision](collision-inventory.md), [input](input-inventory.md)
and [audio](audio-inventory.md) inventories against code. The
[compatibility policy](compatibility.md) explains how API changes are reviewed.

The [upstream catalog](inventory.md) lists BennuGD classic and BennuGD2 exports.
See the [research guide](research.md) and [pinned sources](upstream.md) for
provenance, and the [family review](family-review.md) for the delivered scope.

## Build the Rust API reference

Export `MALVADO_SDK_PREFIX` with the absolute path to an installed graphical
SDK. Run this with the project Rust toolchain:

```sh
RUSTDOCFLAGS="-D warnings" cargo doc --offline --no-deps \
  --workspace --all-features --manifest-path "$MALVADO_SDK_PREFIX/share/malvado/rust/Cargo.toml" \
  --target-dir build/rustdoc
```

Open `build/rustdoc/doc/malvado/index.html` for gameplay or
`build/rustdoc/doc/malvado_sys/index.html` for the native boundary.
