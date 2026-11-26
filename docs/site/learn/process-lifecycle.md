# Process lifecycle and families

When a level ends, its enemies and effects should stop. The controller and
the music can carry on into the next level. Use process ownership to decide
which processes end together.

## End a level

Install the [desktop SDK](../../start/installation/), download `level.lua`,
and run this command in the download directory.

```sh
malvado run --headless level.lua
```

<!-- example -->

This example models the music as a process without playing audio. It exits
after the controller stops that process.

```text
Level complete
Level: returned
Enemy: dead
Music: running
Music: killed
```

## Follow the lifetime

A spawn creates an instance and gives you a handle. An instance can run when
the scheduler gives it a turn. Yielding leaves it alive and able to resume.

```text
spawn → first turn
           ↓
        frame() → next eligible turn
           ↑               │
           └─ yield again ─┘

return or kill → dead → cleanup when Malvado regains control
```

Returning from a Lua process ends it with reason `"returned"`. Killing a
live process ends it with reason `"killed"`. If a running Lua process kills
itself, execution stops at that call. Cleanup follows when the scheduler
regains control. Malvado releases pending messages and the process's own
references. A resource can stay alive if another part of the game holds it.

Keep a handle if you need to inspect the outcome later. In Lua, you can still
read `id`, `status`, and `reason` after the process ends. Game fields become
unavailable and writes raise an error. Use `mv.process.alive(handle)` to
check a handle before using its live fields. Sleep and freeze suspend turns
while the process stays alive.

## Follow ownership at teardown

A process spawned from the game's initializer is a root and has no owner.
A running Lua process normally owns the children it starts. An attached child ends
when its owner ends by return or kill. This also applies to the child's
attached descendants. The example's controller starts the level, and the
level starts both the enemy and the music.

| Process | Owner while the level runs | When the level returns |
| --- | --- | --- |
| Controller | None | Continues and checks the results. |
| Level | Controller | Ends naturally. |
| Enemy | Level | Ends with the level. |
| Music | None. The level created it with `attached = false`. | Continues until the controller kills it. |

The music process still has the level's ID in `creator_id`, even though its
`parent` is `nil`. This matters when you choose how to stop a group. A plain
`mv.kill(level)` also ends attached descendants. By default,
`mv.kill_tree(level)` follows creation history and can also reach detached
descendants. Use `{relation = "ownership"}` with a live root to follow only
ownership. Returning from the level is enough to clean up its attached enemy.

## Try a change

Replace the music spawn with `music = Music()`. Run the example again and
look for `Music: dead` before the controller's kill call. The music is now
attached and ends with the level. Calling `mv.kill` on that ended handle
returns `false`.

## Learn more

- [Lua children and ownership](https://github.com/jepemo/malvado/blob/main/docs/api/lua-processes.md#children-and-ownership),
  [process fields](https://github.com/jepemo/malvado/blob/main/docs/api/lua-processes.md#process-data),
  and [control operations](https://github.com/jepemo/malvado/blob/main/docs/api/lua-processes.md#control-processes).
- [Retained C identities](https://github.com/jepemo/malvado/blob/main/docs/api/c-contract.md#keep-a-process-identity)
  and [Rust handles (experimental)](https://github.com/jepemo/malvado/blob/main/docs/api/rust-processes.md#state-tokens-and-handles).
- [Organize a level](../../patterns/level-lifecycle/) and [learn how often a process runs](frames-and-time.md).
