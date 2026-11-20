# Gameplay API guides

C callbacks, Lua coroutines and Rust futures use the same process scheduler.
Use the [API reference][reference] to find the exact public names, types and
platform requirements.

## Run and stop processes

| Language | Start and advance | Finish or cancel |
| --- | --- | --- |
| C | Create an `MvContext`, add callbacks and drive ticks from the host. | Return a completion result or send a kill signal. Release owned handles and destroy the context after the last tick. See the [C contract][c]. |
| Lua | Define processes with `mv.process` and start them with `mv.start` or in a returned game table. Call `frame()` inside a process to yield. | Return from the process or signal it using its handle. The runner handles shutdown. See [Lua processes][lua]. |
| Rust | Use `Game::new` and `Game::spawn`. Drive native ticks from the host. Call `process.frame().await` inside the future. | Return `Ok(())` or send a kill signal. Dropping `Game` cancels any remaining processes. See [Rust processes][rust]. |

Cancel a process by its handle. An owned C or Rust handle can record the end
reason after the process ends. Attached children end with their owner. A detached
child can continue until its own completion or game shutdown. See the
[process cleanup contract][cleanup] and [runner timing][timing].

## Organize a level

Use one process for the game controller and one for the active level.
Start the level's enemies and effects from the level process so they end
with it. Keep the score and other state that lasts across levels in the
controller. End the current level before starting the next one. A process
created during a tick starts on the next tick.

The [process tree example][process-tree] shows the lifetime pattern in
Lua. The controller starts a group with two children, then calls
`kill_tree` with the group handle. The same pattern can clean
up a level when the player finishes it or returns to a menu.

## Choose a reference

| Task | C and Lua reference | Rust reference |
| --- | --- | --- |
| Open a window, draw and read input | [Platform functions][platform], [drawing][drawing], [keyboard and text input][keys] | [Window][rust-window], [drawing][rust-drawing], [input][rust-input] |
| Load assets and animate sprites | [Asset functions][assets], [images][images], [animations][animation] | [Atlas][rust-atlas], [FPG][rust-fpg], [cameras and animation][rust-animation] |
| Use text and fonts | [Text][text] | [Fonts][rust-font] |
| Move, follow and check collisions | [Cameras][cameras], [collision and worlds][collision] | [Cameras][rust-animation], [collision and worlds][rust-collision] |
| Play sounds and music | [Audio][audio] | [Rust audio][rust-audio] |

Use the [C headers][c-api] for signatures and status values. Use the
[Lua flat exports][lua-globals] and [namespace reference][lua-namespaces] for
module names, constants and methods. Build the Rust API reference locally with
the [installed SDK command][rust-reference].

## Keep resources alive

Release direct C image, font and audio objects before closing the service that
created them. Pair each managed retain with a release. In Lua, a reachable
userdata value or queued command can keep the native resource alive after the
process ends. In Rust, safe wrappers retain the service they need and release
their references when dropped. See [resource lifetime][resources] for sharing,
transfer, failure and cleanup rules.

## Choose a runtime and target

Run scheduler-only code with the C core or the default Rust crate. Use
`malvado run --headless` for a Lua project that needs no window or audio
device. The Lua runner owns its loop. C and Rust callers drive their own
input, audio and rendering.

Use a matching desktop SDK for native projects. The Android CLI export target
is arm64 from macOS or Linux. See [SDK and projects](sdk.md#export-for-android)
and the [platform matrix][targets] for format, toolchain and device limits.
The [language matrix][languages] describes which features each API provides.

[reference]: https://github.com/jepemo/malvado/blob/master/docs/api/README.md
[rust-reference]: https://github.com/jepemo/malvado/blob/master/docs/api/README.md#build-the-rust-api-reference
[c]: https://github.com/jepemo/malvado/blob/master/docs/api/c-contract.md
[c-api]: https://github.com/jepemo/malvado/blob/master/docs/api/current-c-api.md
[lua]: https://github.com/jepemo/malvado/blob/master/docs/api/lua-processes.md
[lua-globals]: https://github.com/jepemo/malvado/blob/master/docs/api/lua-globals.md
[lua-namespaces]: https://github.com/jepemo/malvado/blob/master/docs/api/lua-namespaces.md
[rust]: https://github.com/jepemo/malvado/blob/master/docs/api/rust-processes.md
[cleanup]: https://github.com/jepemo/malvado/blob/master/docs/api/process-cleanup.md
[timing]: https://github.com/jepemo/malvado/blob/master/docs/api/runner-timing.md
[process-tree]: https://github.com/jepemo/malvado/tree/master/examples/process-tree
[platform]: https://github.com/jepemo/malvado/blob/master/docs/api/platform-inventory.md
[drawing]: https://github.com/jepemo/malvado/blob/master/docs/api/drawing.md
[keys]: https://github.com/jepemo/malvado/blob/master/docs/api/keyboard.md
[assets]: https://github.com/jepemo/malvado/blob/master/docs/api/asset-inventory.md
[images]: https://github.com/jepemo/malvado/blob/master/docs/api/images.md
[animation]: https://github.com/jepemo/malvado/blob/master/docs/api/animations.md
[text]: https://github.com/jepemo/malvado/blob/master/docs/api/text.md
[cameras]: https://github.com/jepemo/malvado/blob/master/docs/api/cameras.md
[collision]: https://github.com/jepemo/malvado/blob/master/docs/api/collision.md
[audio]: https://github.com/jepemo/malvado/blob/master/docs/api/audio.md
[resources]: https://github.com/jepemo/malvado/blob/master/docs/api/resource-lifetime.md
[rust-window]: https://github.com/jepemo/malvado/blob/master/docs/api/rust-window.md
[rust-drawing]: https://github.com/jepemo/malvado/blob/master/docs/api/rust-drawing.md
[rust-input]: https://github.com/jepemo/malvado/blob/master/docs/api/rust-input.md
[rust-atlas]: https://github.com/jepemo/malvado/blob/master/docs/api/rust-atlas.md
[rust-fpg]: https://github.com/jepemo/malvado/blob/master/docs/api/rust-fpg.md
[rust-animation]: https://github.com/jepemo/malvado/blob/master/docs/api/rust-camera-animation.md
[rust-font]: https://github.com/jepemo/malvado/blob/master/docs/api/rust-font.md
[rust-collision]: https://github.com/jepemo/malvado/blob/master/docs/api/rust-collision.md
[rust-audio]: https://github.com/jepemo/malvado/blob/master/docs/api/rust-audio.md
[targets]: https://github.com/jepemo/malvado/blob/master/docs/development/platforms.md
[languages]: https://github.com/jepemo/malvado/blob/master/docs/api/language-parity.md
