# Organize a level

When a player reaches the next level, the old enemies and effects should
end while the score stays visible. Let a game controller own the HUD and
start a separate process for each level. Let the level own its actors.

## Organize a level

The lines below show ownership. The level creates the ambience separately
as a detached process, so it has no owner. Malvado still records the level
as its creator.

```text
Controller (persists between levels)
├── HUD (attached to Controller)
└── Level (attached to Controller)
    └── Enemy (attached to Level)
        └── Effect (attached to Enemy)

Ambience (detached root, created by Level)
```

## Run two levels

Install the [desktop SDK](../../start/installation/) and download
`level-lifecycle.lua`. From the download directory, run the program below.
It checks the process relationships and exit behavior on both levels, then
exits. The HUD and ambience stand in for display and audio processes.
The program needs no window, input, audio or assets.

```sh
malvado run --headless level-lifecycle.lua
```

<!-- example -->

```text
Level 1 cleared
HUD score: 10
Level 2 cleared
HUD score: 20
```

## End the level before starting another

The controller waits for the level to return. This ends the enemy and its
effect through the ownership tree. The HUD stays alive because its owner,
the controller, is still running. It owns the score and acknowledges each
update so the controller can confirm the score carries over.
The effect uses a Lua `<close>` variable to let the controller check that
teardown actually ran.

The detached ambience survives level teardown. A game could let ambience
finish a transition before ending it. In this example the controller keeps
its handle and explicitly stops it before starting the next level.
Attach level work by default. Detach it only when another part of the game
takes responsibility for ending it.

Returning from the level or calling `mv.kill(level)` ends its attached
subtree. `mv.kill_tree(level)` follows creation history by default and can
also kill detached descendants. Use the ownership relation when that is
the tree you intend to target. Process teardown releases references owned
by those processes. Resources with other live references can survive, so
keep shared assets with the part of the game that uses them across levels.

For a single scene that ends with the game, one controller may be enough.
A level process gives you a lifetime boundary when scenes need to be replaced.

## Keep learning

- [Learn how process families end](../../learn/process-lifecycle/).
- [Learn what retains resources](../../learn/resources/).
- [Check Lua child ownership](../../concepts/lua-processes/#children-and-ownership).
- [See the Lua kill-tree reference](../../reference/lua/processes/kill_tree/).
- [Try the process tree example](https://github.com/jepemo/malvado/tree/main/examples/process-tree).
