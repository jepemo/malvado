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
