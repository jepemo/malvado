# Lua API reference migration

The [global exports](../api/lua-globals.md) and
[namespace inventory](../api/lua-namespaces.md) define the function surface.
The [instance method inventory](../api/lua-instance-methods.md) defines the
optional method forms. Callable namespaces retain their own entries. A flat
function and its namespace alias share a page when they already share a route.
Methods appear with their explicit procedural operation.

Each row covers the function routes in an independently reviewable family.
Every listed entry includes its call forms, return contract and a complete
program. The generator prints current counts from the entries it publishes.

| Family | Entries | Runnable programs |
| --- | ---: | --- |
| Processes | 20/20 | Process control, two workers, mailbox, collision |
| Input | 14/14 | Input monitor |
| Graphics | 8/8 | Clipped graphics, atlas |
| Drawing | 6/6 | Shapes |
| Images and sprites | 15/15 | Images, atlas |
| Animations | 1/1 | Atlas |
| Text | 6/6 | Persistent text |
| Audio | 15/15 | Audio playback |
| Cameras | 6/6 | Cameras |
| Collision | 6/6 | Collision |
| Worlds | 6/6 | Static world |
| FPG | 19/19 | FPG metadata, FPG image |
| Platform | 13/13 | Startup, window title, fades, dice roll, random seed |
| Module | 2/2 | Global import, legacy sheet |

The method inventory maps 65 methods to these entries. Family and guide links
cover the associated constants, types and process properties. The
[reference examples](../../examples/lua-reference/README.md) list their files
and commands. The website also publishes existing programs for
[two workers](../../examples/features/processes/README.md),
[mailbox handling](../../examples/process-lesson/README.md), and
[a dice roll](../../examples/rand/README.md).

## Verification limits

The input monitor can run on desktop without a controller. Checking actual
controller mapping and Android on-screen keyboard behavior requires a
supported device or emulator. The native input tests provide deterministic
coverage of snapshots and input validation. The export setup on the
keyboard reference explains how to run the Android portion.

Versioned reference pages link to the current runnable program when its source
was added after the selected revision. The snapshot publishes downloads from
its own revision when those files exist.

Broader coverage enforcement and bounded example execution in CI are
tracked in [Docs 11](https://github.com/jepemo/malvado/issues/71).
