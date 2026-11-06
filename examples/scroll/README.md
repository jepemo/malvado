# Process scrolling

Run from the repository root:

```sh
./build/malvado run examples/scroll/main.lua
```

Use the arrow keys to move the view. The blue and orange shapes use world
coordinates and are clipped to the outlined region. The text and outline stay
in window coordinates. Press Escape to exit. Scroll does not move colliders.
See the [scroll contract](../../docs/api/drawing.md#scroll-a-lua-process).
