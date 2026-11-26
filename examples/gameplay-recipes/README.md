# Gameplay recipes

These standalone Lua programs teach common tasks with Malvado processes.
Install a [desktop SDK](../../docs/site/getting-started.md#install-the-sdk)
and run the commands from this directory. You can also copy any script to
a fresh directory and run the same command there. The programs print a
trace and exit on their own. They use no controls or external assets.

| Command | Expected behavior | Recipe |
| --- | --- | --- |
| `malvado run --headless game-states.lua` | Enter play, pause without losing time, resume and reach game over. | [Manage game states](../../docs/site/patterns/game-states.md) |
| `malvado run --headless finite-state-machine.lua` | The guard patrols, chases, rests and recovers. | [Give an actor a finite state machine](../../docs/site/patterns/finite-state-machine.md) |
| `malvado run --headless spawn-enemies-and-bullets.lua` | One bullet and one enemy reach opposite edges before the wave ends. | [Spawn enemies and bullets](../../docs/site/patterns/spawn-enemies-and-bullets.md) |
| `malvado run --headless coordinate-processes.lua` | A frozen door waits for wakeup, then opens and acknowledges the request. | [Coordinate processes with messages and signals](../../docs/site/patterns/coordinate-processes.md) |
| `malvado run --headless level-lifecycle.lua` | Two levels end with their attached actors. The controller stops detached ambience and the HUD keeps score across levels. | [Organize a level](../../docs/site/patterns/level-lifecycle.md) |

These recipes provide Lua source. See [Thinking in processes](../../docs/site/learn/process-model.md)
for equivalent scheduler examples in C, Lua and experimental Rust.
