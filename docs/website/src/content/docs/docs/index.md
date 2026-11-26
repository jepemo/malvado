---
title: Introduction
description: Learn to write games with Malvado processes.
---

The unversioned guides follow current development. Use the
[changelog](https://malvado.dev/docs/changelog/) to find the documentation
for a published SDK. The site title shows the version on each snapshot.

Malvado is a 2D game engine inspired by DIV Games Studio. The Lua
starter opens a window and draws a greeting near the upper left.

```lua
draw_text {text = "Hello, Malvado!", x = 40, y = 40, size = 30}
```

This line runs inside a **process**. A process is a small piece of the
game that can keep its own state and wait for its next turn. You can
give the player, each enemy, and each effect its own process. When the
player waits for the next frame, the enemies still take their turns.

Write games in Lua, C, or experimental Rust. Use the desktop SDK on
macOS arm64, Linux x86_64, or Windows x64. See
[platform support](./tools/platforms/) for desktop and Android exports.

- [Install the SDK](./start/installation/) and [change the greeting in your first Lua game](./start/first-program/).
- [Learn how processes work](./learn/process-model/) or [try runnable examples](./examples/).
- [Choose an API reference](./reference/) for Lua, C, or experimental Rust.
