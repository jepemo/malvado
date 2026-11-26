# Thinking in processes

A player reads input, an enemy follows the player, and a bullet moves until
it hits something. Give each of these behaviors its own process. A controller
can start them, keep the score, and decide when to start the next level.

## Run two workers

Install the [desktop SDK](../../start/installation/). Download `main.lua` and
open a terminal in the download directory. This program prints four lines
and exits on its own. It needs no window or assets.

```sh
malvado run --headless main.lua
```

<!-- example -->

## Follow each turn

`mv.process(function(self) ... end)` defines the behavior once. Each call to
`worker` creates an instance with its own `self.name`, local variables,
and place in the function. The game starts both instances during setup.

| Tick | First worker | Second worker |
| --- | --- | --- |
| 1 | Prints `first: 1`, then yields at `mv.frame()`. | Prints `second: 1`, then yields. |
| 2 | Resumes after `mv.frame()`, prints `first: 2`, then returns. | Resumes, prints `second: 2`, then returns. |

Read each row from left to right for the terminal trace. In Lua, the scheduler
runs one process at a time. A call to `mv.frame()` lets another process take
its turn. When this instance is next due, execution picks up after the call.
A loop that never yields or returns keeps the scheduler from advancing.

With a manually updated entity list, the game loop selects each entity and
calls its update function. Here, Malvado schedules eligible processes and
each process can express a sequence of actions across ticks in its own
function. An enemy can approach, wait, and retreat in one function.

Higher-priority processes get their turns first. When priorities match,
eligible processes run in creation order. Both workers here have the same
priority and run every tick. A priority change during a tick applies to the
order of the next tick. Processes can skip turns due to cadence, sleep, or
freeze. A process spawned during a tick can first run on the next tick or
later.

## Playground

<!-- playground -->

## Try a change

Swap the two calls to `worker` in `init`. Which name will appear first on each
tick? Run it again and check that `second` now appears before `first` on both
ticks. The two instances still resume at their own calls to `mv.frame()`.

## Run and stop processes

Lua, C, and experimental Rust use the same process scheduler. Each language
has its own way to give up a turn and end a process.

| Language | Take a turn | Finish or cancel |
| --- | --- | --- |
| Lua | Run a process function and call `mv.frame()` to yield. | Return or use `mv.kill(handle)`. |
| C | Drive ticks from the host. Each step callback returns `MV_STEP_YIELD` to take another turn on a later tick. | Return `MV_STEP_DONE` or send a kill signal. Release retained handles and destroy the context when finished. |
| Rust (experimental) | Spawn an async function with `game.spawn` and use `p.frame().await`. | Return `Ok(())` or send a kill signal. Dropping `Game` cancels remaining processes. |

Try the [same worker lesson in C and experimental Rust](https://github.com/jepemo/malvado/blob/main/examples/features/processes/README.md).
See [Lua process definitions](https://github.com/jepemo/malvado/blob/main/docs/api/lua-processes.md#start-and-stop),
the [Lua `frame()` reference](../../reference/lua/processes/frame/),
the [C process API](https://github.com/jepemo/malvado/blob/main/docs/api/c-contract.md#processes-and-signals),
and [Rust activation and cancellation](https://github.com/jepemo/malvado/blob/main/docs/api/rust-processes.md#activation-and-cancellation).
Continue with [process lifecycle and families](process-lifecycle.md).
