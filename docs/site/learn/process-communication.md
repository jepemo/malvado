# Process communication

When a player picks up a coin, the player's process can send the points to
a scoreboard. The scoreboard owns the total and chooses when to read its
mailbox. The same approach lets several processes report events to a
controller that owns the round state.

## Send points to a scoreboard

Install the [desktop SDK](../../start/installation/), download `mailbox.lua`,
and run it from the download directory. The program prints the total after
each pickup and exits when the scoreboard receives the end-of-round message.

```sh
malvado run --headless mailbox.lua
```

<!-- example -->

```text
Score: 10
Score: 15
Final score: 15
```

## Read the mailbox on your turn

Each process has its own queue. `mv.send(handle, value)` adds a value to the
recipient's queue. `mv.recv()` inside a running process removes the oldest
message from its own queue. An empty queue returns `nil` immediately. The
scoreboard yields to let the player run, then reads one value on each turn.

The program uses `false` for the end of the round. Check for `nil` when you
need to tell an empty queue apart from a queued `false`. Lua rejects
sending `nil`, a send to an ended process, or a send to a full queue. A queue
can hold 1,024 messages and its contents are released when its process ends.
Between Lua processes, a table message keeps its identity. Treat shared
tables as read-only messages or make a copy when you need a snapshot.

## Use a signal to change whether a process runs

Signals change a process's execution state. Use `mv.signal(target,
mv.SIGNAL_SLEEP)` to suspend turns and hide the process's last drawing.
`mv.SIGNAL_FREEZE` suspends turns and keeps the last drawing visible.
`mv.SIGNAL_WAKEUP` makes a suspended process eligible again.
`mv.SIGNAL_KILL` ends it. Sending a message queues data without waking
a suspended recipient. Use wakeup when the recipient needs to run again.

## Try a change

Swap the two numeric sends and run again. The first total is now 5 and the
final total is still 15. Next, replace the final send with a different kind of
message and update the receiver to recognize it. Keep a distinct
end-of-round value so the scoreboard can finish.

## Learn more

See Lua [mailboxes](https://github.com/jepemo/malvado/blob/main/docs/api/lua-processes.md#send-and-receive-messages)
and [signals](https://github.com/jepemo/malvado/blob/main/docs/api/lua-processes.md#control-processes).
For C, see [message send, receive, and ownership](https://github.com/jepemo/malvado/blob/main/docs/api/c-contract.md#exchange-messages).
Experimental Rust exposes [process signals](https://github.com/jepemo/malvado/blob/main/docs/api/rust-processes.md#activation-and-cancellation);
its safe gameplay API has no mailbox operations.
See [Organize a level](../../patterns/level-lifecycle/) for a controller
that coordinates a level through ownership.
