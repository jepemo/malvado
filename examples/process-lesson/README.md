# Process lessons

Use a desktop Malvado SDK to run these Lua examples. They use no assets, window,
or controls. Each exits when its processes finish.

| Command from this directory | What to check |
| --- | --- |
| `malvado run --headless level.lua` | The level returns. Its enemy dies and the independent music process stays running until the controller kills it. The music process represents playback lifetime and plays no audio. |
| `malvado run --headless movement.lua` | Both movers travel 12 logical pixels in 0.10 logical seconds at different activation rates. |
| `malvado run --headless mailbox.lua` | The scoreboard receives 10, 5, and the end-of-round message in order. It ends with a score of 15. |

For the runnable two-worker lesson in C, Lua, and experimental Rust, see
[Processes](../features/processes/README.md).

These sources use the project [Apache 2.0 license](../../LICENSE).
