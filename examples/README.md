# Malvado examples

Examples show how to use Malvado in C, Lua, and Rust. The selected teaching
collection will contain small feature examples and a few compact minigames.
Each topic must have aligned behavior, simple idiomatic code, useful English
comments, and a shared README for all three languages.

Start with the [aligned lessons](features/README.md) for startup,
process scheduling, image loading, and window resizing.

The directory migration is planned, not complete. Current entry points include:

- [C processes and its Lua counterpart](c/processes/README.md).
- [Lua process lessons](process-lesson/README.md) on teardown, movement timing, and mailboxes.
- [Lua Meteors scene](meteors/README.md) with three independent processes.
- [Lua Move scene](move/README.md) with an arrow-key circle.
- [Catch! game](catch/README.md) in C, Lua and Rust.
- [Dodge! tutorial checkpoints](dodge/README.md) for a first Lua game.
- [Lua gameplay recipes](gameplay-recipes/README.md) for common process-based tasks.
- [Lua blinking text](blinking-text/README.md) with shared fonts and RGBA colors.
- [Lua world collision](world-collision/README.md).
- [Lua Space Asteroids game](space_asteroids/README.md).
- [First Rust games](rust/README.md) with Hello World and independent moving balls.
- [Current Rust process and world SDK examples](../bindings/rust/README.md).

Each example's README gives its commands and language coverage. See the
[catalog proposal](../docs/roadmap/examples.md) for the planned layout and
migration.
