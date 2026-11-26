# Dodge!

Move a cyan paddle left and right to avoid falling hazards. This Lua
tutorial teaches movement, timed spawning, collision, and round cleanup
using Malvado processes. The existing [Catch! example](../catch/README.md)
teaches catching and scoring in C, Lua, and experimental Rust. Dodge!
uses survival time and restart to teach how attached processes end with
their round.

## Run a checkpoint

Use a graphics-capable Malvado desktop SDK and a desktop session. In your
projects directory, create a new project.

```sh
malvado new dodge
cd dodge
```

Save any of these complete Lua files as `main.lua` in the new project.

| Checkpoint | Expected result |
| --- | --- |
| [Create the game](01-start.lua) | The window says **Dodge!**. |
| [Draw the player](02-player.lua) | A cyan paddle sits near the bottom. |
| [Move the player](03-movement.lua) | The arrow keys move the paddle within the window. |
| [Add one enemy](04-enemy.lua) | One orange circle falls and disappears. |
| [Spawn enemies](05-spawn.lua) | Several circles fall from different positions. |
| [Detect a hit](06-collision.lua) | Contact removes the player and enemies. |
| [Track survival time](07-score.lua) | A timer counts up until contact. |
| [Restart the game](main.lua) | Game over shows the final time and **Space** starts a new round. |

From the new project directory, run:

```sh
malvado check
malvado run
```

Use **Left** and **Right** to dodge. Press **Escape** or close the window
to stop. The final game uses no asset files. The code uses the project
[Apache 2.0 license](../../LICENSE).

Read the [full tutorial](https://malvado.dev/docs/tutorials/dodge/) for
the code walkthrough and desktop export steps.
