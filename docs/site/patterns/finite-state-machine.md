# Give an actor a finite state machine

A guard patrols until it gets near a target, then chases and stops to rest.
The guard can do one of these jobs at a time. A finite state machine gives
each job a name and makes the conditions for switching jobs explicit.

## Run the guard

Install the [desktop SDK](../../start/installation/) and download
`finite-state-machine.lua`. From the download directory, run the program
below. It uses a fixed target and exits after the guard recovers. It needs
no window, input or assets.

```sh
malvado run --headless finite-state-machine.lua
```

<!-- example -->

```text
Patrol
Chase from 2
Rest at 4
Guard recovered
```

## Keep decisions with the actor

The guard chooses one branch each turn. It moves one unit while patrolling
and switches to chase when it comes within two units of the target. The chase
branch runs on the guard's next activation. It moves two units and switches
to rest at the target. A two-turn rest ends the example.

The state, position and rest counter live in the guard's process.
Each instance can make its own decisions. If the guard could rest again
after recovery, reset the rest counter each time it enters rest. Try changing
the patrol threshold from two units to one and observe where the chase starts.

Explicit states help when an actor has several distinct behaviors and each
transition has a rule. A hazard that moves in one direction needs only a
movement loop. Keep this guard's branches together while they fit in one
function. A general state machine library would add work to this example.

## Keep learning

- [See how processes keep their own state](../../learn/process-model/).
- [Read the Lua process timing rules](../../concepts/lua-processes/#process-timing).
- [Manage game-wide screens and pause](../game-states/).
