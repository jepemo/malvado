# Rust camera and animation values

These services use the installed C core. They need no graphics backend. They do
not select a process camera, load graphs, or update a process automatically.

## Cameras

```rust
use malvado::camera::Camera;
let mut camera = Camera::new(0.0, 0.0, None)?;
camera.center((200.0, 150.0), (800, 600))?;
let screen = camera.project((210.0, 160.0), (0.0, 0.0))?;
assert_eq!(screen, (410.0, 310.0));
# Ok::<(), malvado::Error>(())
```

`Camera` owns a copied native value. Copying it creates an independent camera.
`None` in the constructor uses the full window. An explicit `Viewport` must have
positive dimensions. Coordinates must be finite and within `FLT_MAX / 64`.
Projection applies world minus camera minus local scroll plus viewport origin.
`clip` intersects the viewport, optional region, and screen without projection;
it returns `None` for an empty intersection. Screen dimensions must be positive.
Invalid inputs return `Error::InvalidArgument` and preserve camera state.
No allocation or cleanup is required. The game controls when to center or move
its camera; these values do not add an update loop.

## Animations

```rust
use malvado::animation::{Animation, Frame};
let animation = Animation::new(&[
    Frame { graph_id: 1, duration_us: 100_000 },
    Frame { graph_id: 2, duration_us: 100_000 },
], true)?;
let mut playback = animation.playback();
// Call once in the game's existing simulation tick.
let sample = playback.advance(60)?;
assert_eq!(sample.graph_id, 1);
# Ok::<(), malvado::Error>(())
```

The definition copies 1..8192 frames into native storage and frees it on drop.
Graph IDs must be positive; durations are 1..3600000000 microseconds. Invalid
frames return `InvalidArgument`; native allocation failure returns `OutOfMemory`.
Graph IDs do not load or retain graphical assets. Definitions stay on their
creating thread and can be held by the existing `Resource<Animation>` manager.

`sample(elapsed_us)` samples without changing state. Each `playback()` starts at
zero and borrows its definition, so it cannot outlive the native allocation.
`advance(rate)` calls the native fixed-tick accumulator; rates are 1..1000000.
Fractional microseconds carry across ticks without drift. Call `reset()` before
changing rate. Invalid rates leave state unchanged. Looping playback wraps;
non-looping playback holds its last frame. This service does not assign a graph
or advance automatically when a process runs, sleeps, or freezes.

The Rust tests check boundaries, native frame copying, invalid definitions,
independent playback, exact tick accumulation, rate errors, loop/end behavior,
and retention after context shutdown. Compile-fail examples verify playback
lifetime and owner-thread destruction. Installed-header probes check every
camera/animation signature and all public record layouts and offsets.

Local verification: `mise run test-rust` passes in Debug and Release, including
51 workspace tests, 11 doctests, Clippy, format, bounded existing examples,
SDK rejection fixtures, and an offline external consumer that uses both modules
from a relocated installed SDK. `mise run test-rust-sanitize` passes with the
new native operations; Rust itself is not sanitizer-instrumented.
