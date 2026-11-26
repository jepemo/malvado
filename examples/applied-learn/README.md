# Applied Learn examples

These Lua examples use the desktop SDK and a display. Open a terminal in this
directory, then run one of the asset-free programs.

| Command | What to look for |
| --- | --- |
| `malvado run shapes.lua` | An outlined rectangle, a line and a solid circle. |
| `malvado run movement.lua` | Use the arrow keys to move, Space to center the circle and a left click to place a marker. |
| `malvado run labels.lua` | Press Space to increase the score. Both labels stay centered. |

`labels.lua` needs text alignment support from a build newer than 2.1.0.
See the [development SDK instructions](../../docs/site/sdk.md#install-a-local-development-build).

To run `sprites.lua`, copy the shared atlas into its expected directory.
The same files are included in the website's example ZIP.

```sh
mkdir -p assets
cp ../aseprite/sheet.json ../aseprite/sheet.png assets/
malvado run sprites.lua
```

Two processes share the same sprite. The sprite on the right is rotated.
Press Escape or close the window to exit any of these programs.

The atlas was made for Malvado. It and the source files use the project
[Apache 2.0 license](../../LICENSE). The
[example roadmap](../../docs/roadmap/examples.md) describes the work to align
the teaching catalog across Lua, C, and Rust.
