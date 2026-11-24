# Blinking text

Three texts share one built-in font while each keeps its own size and RGBA
color. The game sets a default of 30 process activations per second. Each
activation updates the text opacity. Press **Escape** to close the window.

The example shows:

- Named-option tables for `font`, `write` and `change_text`.
- One shared font and three independently styled texts.
- RGBA colors as `{red, green, blue, alpha}` tables.
- Per-text size overrides.
- Alpha animation through partial `change_text` updates.
- A default process cadence set by `fps` in the returned game table.

Build Malvado, then run:

```sh
./build/malvado run examples/blinking-text/main.lua
```

The example requires a display or virtual display.
