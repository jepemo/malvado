# Your first game: Dodge!

Move a cyan paddle out of the path of falling orange hazards. The longer you
last, the higher your time. When you get hit, press **Space** to try again.

You will give the player and each hazard its own process. A round process
will spawn hazards, check for hits, and end their turns when the round ends.
The game uses built-in shapes, colors, and text, so you only need Lua source.

## Create and run the project

[Install the desktop SDK](../../start/installation/) and open a terminal in
the directory where you keep projects. Choose an unused directory name for
the project. These commands use `dodge`.

```sh
malvado new dodge
cd dodge
malvado run
```

The starter opens a window that says **Hello, Malvado!**. Press **Escape**
or close the window to return to the terminal.

Download the first checkpoint and save its contents as `main.lua` in
`dodge`, replacing the starter. From that directory, run:

```sh
malvado check
malvado run
```

The window now says **Dodge!**. A process calls `draw_text` each turn and
then calls `frame()` to let the game advance. The table at the end of
`main.lua` sets the window size and starts this process in `init`.
`require("malvado").import_globals()` makes names such as `draw_text` and
`frame` available in the script.

```lua
draw_text {text = "Dodge!", x = 20, y = 20, size = 30}
frame()
```

To resume at any step, save that step's download as `dodge/main.lua` and
run the same two commands from `dodge`. Each download contains the whole
program. The generator's editor files can stay in place. The only required
game file is `main.lua`. The checkpoint code uses the
[Apache 2.0 license](https://github.com/jepemo/malvado/blob/main/LICENSE).

<!-- checkpoint: 01-start -->

## Draw the player

Replace the greeting process with `Player`. Give its definition a default
`graph` so the engine draws the same rectangle every turn. Start the player
in `init`, halfway across the window and 40 pixels above the bottom.

```lua
local Player = process(function(self)
    while not key(KEY_ESCAPE) do
        frame()
    end
end, {defaults = {graph = rect {width = PLAYER_WIDTH, height = 16, color = CYAN}}})
```

Run the checkpoint. You should see one cyan rectangle with its center at
`(320, 320)`. Try changing the initial `y` to `100` and watch it move up.
[Drawing and coordinates](../../learn/drawing-and-coordinates/) explains
how the window axes and the center of a shape work.

<!-- checkpoint: 02-player -->

## Move with the arrow keys

In the player's loop, read the left and right keys, then update `self.x`.
Multiplying 300 pixels per second by `self.delta` gives the distance for
this update. Keep the center at least half a paddle width from either edge
so the whole paddle stays visible.

```lua
local distance = 300 * self.delta
if key(KEY_LEFT) then self.x = self.x - distance end
if key(KEY_RIGHT) then self.x = self.x + distance end
local half_width = PLAYER_WIDTH / 2
self.x = math.max(half_width, math.min(WIDTH - half_width, self.x))
```

Run the checkpoint. Hold **Left** or **Right** until the paddle stops at the
edge. It stops moving when you release the key.
[Input and movement](../../learn/input-and-movement/) covers held keys and
[Frames and time](../../learn/frames-and-time/) explains `self.delta`.

<!-- checkpoint: 03-movement -->

## Give one enemy its own process

Add an `Enemy` definition with an orange circle as its default graph. Start
one enemy at the top center in `init`, after starting the player. Increasing
`self.y` moves the enemy down. Once the full circle moves below the window,
the loop ends and the process returns.

```lua
while not key(KEY_ESCAPE) and self.y < HEIGHT + ENEMY_RADIUS do
    self.y = self.y + 140 * self.delta
    frame()
end
```

Run the checkpoint. The orange circle falls, passes through the paddle, and
disappears. You can move the paddle while it falls. Each process updates its
own position, then waits for its next turn.
[Thinking in processes](../../learn/process-model/) shows how those turns fit
together.

<!-- checkpoint: 04-enemy -->

## Spawn enemies over time

Start a `Round` process in `init` and let it start the player. Give the round a
list of horizontal positions and a countdown that starts at zero. On each turn,
subtract the time since its last update. When the countdown reaches zero,
create another enemy above the window and move to the next position in the
list. The round now handles Escape for both the player and enemies.
Adding 0.7 seconds to the countdown carries over any time beyond the
deadline into the next interval.

```lua
until_spawn = until_spawn - self.delta
if until_spawn <= 0 then
    Enemy {x = lanes[lane], y = -ENEMY_RADIUS}
    lane = lane % #lanes + 1
    until_spawn = until_spawn + 0.7
end
```

Run the checkpoint. Several orange circles can be on screen at once. Each
falls and disappears on its own. Try changing `0.7` to `0.4` to make the
circles come more often.

<!-- checkpoint: 05-spawn -->

## End the round on contact

Keep the handle returned when you start the player. Before spawning, ask
whether the player overlaps any instance of `Enemy`. The rectangle and
circle graphs supply the collision shapes. When the query returns an enemy
handle, return from the round.

```lua
if player:collision(Enemy) then
    return
end
```

Run the checkpoint and leave the paddle in the center. On contact, the paddle
and all enemies disappear together. With the last process gone, the window
closes. Move aside in a second run and the first enemy passes without ending
the round.

The round started the player and enemies from inside its process. They are
attached children by default. When the round returns, Malvado ends its
remaining children. See [Collisions](../../learn/collisions/) and
[Process lifecycle](../../learn/process-lifecycle/).

<!-- checkpoint: 06-collision -->

## Show the survival time

Set `survived` to zero at the start of the round. On each turn that passes
the collision check, add the time since the last update. Call `draw_text`
each turn so the label stays visible.

```lua
survived = survived + self.delta
draw_text {text = ("Time: %.1f s"):format(survived), x = 20, y = 20, size = 24}
```

Run the checkpoint. The timer in the upper left climbs while you dodge and
disappears when the round ends.

<!-- checkpoint: 07-score -->

## Show game over and restart

Start a `Game` process in `init`. It owns the round and stays alive after
the round ends. Pass the round a fresh result table. On contact, the round
writes its final time to that table before returning.

```lua
local result = {}
local round = Round {result = result}
while round.status ~= STATUS_DEAD do
    if key(KEY_ESCAPE) then return end
    frame()
end
```

Once the round has ended, `Game` reads `result.time` from the table it still
holds and draws the game-over message. It waits for a new **Space** press
with `key_pressed(KEY_SPACE)`, then starts the next round. The fresh round
creates a centered paddle and starts its own timer and spawn countdown. The
two process definitions still share their respective shape graphs with
each new instance.

Run the finished game. Let the first hazard hit you and check the final time
on the game-over screen. Press **Space**. A new paddle appears in the center,
the timer starts again, and hazards from the last round are gone. Try to beat
your time. Press **Escape** or close the window to quit.

<!-- checkpoint: main -->

## Share the finished game

Save the finished checkpoint as `main.lua` and run these commands from
`dodge`. The default export destination must be new.

```sh
malvado check
malvado export
```

On macOS or Linux, run `./dist/game/game` from `dodge`. On Windows, run
`.\dist\game\game.exe` in PowerShell. You should see the same game with the
same controls. Share the entire `dist/game` folder, including its license
notices, with someone on a matching operating system and architecture.
They can run the launcher in that folder without the SDK.
[Build and export games](../../tools/export/#export-a-game) covers other
destinations and formats.

## Try a change

Change the hazard speed or the order of the spawn positions and see how
long you can last. When you want to use your own assets, follow the guides
for [sprites](../../learn/graphics-and-sprites/),
[animation](../../learn/animation/), or [sound](../../learn/sound-and-music/).

Try [Catch!](../../examples/catch/) for a different goal. You move toward
falling balls to score points. Its complete example is available in Lua,
C, and experimental Rust.
