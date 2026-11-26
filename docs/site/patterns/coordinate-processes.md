# Coordinate processes with messages and signals

A controller wants to enable an exit after a door opens. Let the door own
its behavior and report when it finishes. A mailbox carries the request and
the reply. A signal changes whether the door process can take a turn.

## Run the door and controller

Install the [desktop SDK](../../start/installation/) and download
`coordinate-processes.lua`. From the download directory, run the program
below. It exits when the controller receives the door's reply. It needs no
window, input or assets.

```sh
malvado run --headless coordinate-processes.lua
```

<!-- example -->

```text
Open request queued
Door opened
Exit ready
```

## Wait for a reply

The controller freezes the door, queues an `open` message and yields.
Sending only adds data to the mailbox. The door stays frozen until the
controller sends a wakeup signal. The door then reads its request on a
scheduled turn, replies with `opened` and returns. The controller polls
its own mailbox and yields between checks. The acknowledgment determines
when the exit is ready, so the controller can work with a door that takes
several turns to open.

A mailbox suits work that happens across process turns. Within one process,
call a Lua function when you need an immediate result. If the door can be
destroyed before it replies, plan what the controller does when the door
ends or a deadline passes. A mailbox is bounded and a send to an ended
process raises an error.

## Keep learning

- [Learn what mailboxes and signals do](../../learn/process-communication/).
- [Check how to send and receive Lua messages](../../concepts/lua-processes/#send-and-receive-messages).
- [See the Lua signal reference](../../reference/lua/processes/signal/).
