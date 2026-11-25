# Aseprite animation

Run from the repository root with a build that includes graphics and assets:

```sh
./build/malvado run examples/aseprite/main.lua
```

The installed CLI also runs the example with `malvado run .` from this
directory or with `malvado run /path/to/main.lua` from any directory.

The example loads a PNG and Aseprite JSON array with `load_asset()`, then assigns
`animation("bounce")` to `self.graph`. The engine advances it automatically.
The frames use different durations, crop offsets, and a shared pivot in the
original canvas. Press Escape to exit. The small PNG and JSON are original
Malvado test assets under the project license. See [animations](../../docs/api/animations.md)
and the [image guide](../../docs/api/images.md#import-an-aseprite-json-array).
