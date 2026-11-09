# Aseprite animation

Run from the repository root with a build that includes graphics and assets:

```sh
./build/malvado run examples/aseprite/main.lua
```

The example loads a PNG and Aseprite JSON array, then samples the `bounce` tag in a process and assigns static `graph()` descriptors.
This example still advances time explicitly; automatic `animation()` playback
is a separate alpha.6 deliverable. The frames use different durations, crop offsets, and a shared pivot in the original canvas. Press Escape to exit. The small PNG and JSON are original Malvado test assets, distributed under the project license. See the [image guide](../../docs/api/images.md#import-an-aseprite-json-array) for supported fields and limits.
