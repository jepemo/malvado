# Parse an atlas in C

This example reads atlas metadata without Lua, a window, or an image. It looks
up a sprite and selects an animation frame. It prints:

```text
ship.png: ship, sprite 7, frame 7
```

From the repository root, build the C-only assets configuration and run it:

```sh
cmake -S . -B build-c-assets -DMALVADO_BUILD_LUA=OFF -DMALVADO_BUILD_RAYLIB=OFF -DMALVADO_BUILD_ASSETS=ON
cmake --build build-c-assets
./build-c-assets/example-c-atlas
```

The parser owns the returned metadata until `mv_atlas_destroy`. It does not load
the image. See the [image guide](../../../docs/api/images.md) for the format,
errors, ownership rules, and Lua use.
