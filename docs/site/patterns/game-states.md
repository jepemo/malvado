# Manage game states

A pause screen should stop the round timer while the game still accepts
a resume action. Let a controller own the game's current mode. Use that mode
to decide which actions affect the round.

## Run the controller

Install the [desktop SDK](../../start/installation/) and download
`game-states.lua`. From the download directory, run the program below.
It plays a fixed sequence of actions and exits at game over. It needs no
window, input or assets.

```sh
malvado run --headless game-states.lua
```

<!-- example -->

```text
menu
play
Time left: 1
pause
play
Time left: 0
game over
```

## Let each mode decide what runs

The controller stores the mode and the round timer across calls to
`mv.frame()`. Only a turn with a `tick` action in `play` counts down.
Each tick action uses one unit from the countdown.
The resume action keeps the time left from before the pause. In a windowed
game, read input transitions in the controller and draw the current screen.

The example gates a timer stored in the controller. Actors with their own
processes keep their own schedule. For a larger game, decide which actors
should stop and use [process signals](../../reference/lua/processes/signal/)
to suspend and resume those processes. For example, freezing gameplay
processes keeps their last process drawing visible. Leave the controller
runnable so it can accept resume input. A process's `delta` includes time
spent suspended, so account for that gap if movement uses `delta`.

One mode variable suits screens that replace each other. If a game has only
a single playable screen and an end condition, a loop followed by the end
screen can be enough. Keep independent choices, such as whether the sound is
muted, in separate fields.

## Keep learning

- [Read input transitions](../../learn/input-and-movement/).
- [See game over and restart in Dodge!](../../tutorials/dodge/#show-game-over-and-restart).
- [Use a finite state machine for an individual actor](../finite-state-machine/).
