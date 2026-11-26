# Spawn enemies and bullets

An enemy and a bullet can enter a game at different times and move at different
speeds. Define one process for each behavior. Have a wave controller start
enemies as they arrive and start bullets when the player fires.

## Run a short wave

Install the [desktop SDK](../../start/installation/) and download
`spawn-enemies-and-bullets.lua`. From the download directory, run the
program below. The wave starts an enemy, waits two turns, then fires one
bullet. Each actor announces when it reaches an edge. The wave exits when
both actors finish. It needs no window, input or assets.

```sh
malvado run --headless spawn-enemies-and-bullets.lua
```

<!-- example -->

```text
Bullet reached the right edge
Enemy reached the left edge
Wave clear
```

## Give each actor a lifetime

Each spawn supplies its own starting position. The enemy and bullet resume
their loops when the scheduler gives them another turn. This handles part of
the Update Method pattern for you. You write the movement and yield with
`mv.frame()`, while the scheduler decides when to resume each process.
The wave controller tracks completion through handles. It has no list of
update callbacks to advance or remove.

A process spawned during a tick starts no earlier than the next tick.
When an actor reaches the edge, it returns. The wave stays alive long enough
for both attached actors to finish. To add more enemies or bullets, start a
process when its spawn timer or fire action calls for one. Keep the
edge check, collision response or another end condition so actors have a
way to leave the wave.

A process fits an actor with its own evolving state and lifetime. A flash
drawn only on the current turn can be a drawing call. With very large waves,
measure the cost of the game before changing how it stores or creates actors.

## Keep learning

- [Learn how the process scheduler shares turns](../../learn/process-model/).
- [Check process activation timing](../../learn/frames-and-time/).
- [See the Lua spawn reference](../../reference/lua/processes/spawn/).
- [See timed enemy spawning in Dodge!](../../tutorials/dodge/#spawn-enemies-over-time).
