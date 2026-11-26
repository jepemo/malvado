# Lua API reference examples

These small programs demonstrate related operations from the
[Lua API reference](https://malvado.dev/docs/reference/lua/). Each function
page links to the complete source and, when needed, a ZIP with its assets.

Install a Malvado SDK. Open a terminal in an empty directory, save the Lua
file there, and run the command in the table. For a ZIP, extract it in that
directory and keep the asset paths shown below. A graphical example needs a
desktop display. Audio also needs an audio device. Atlas and FPG examples need
asset support. All examples exit on their own. You can use the window close
control to stop a desktop example early.

| Program | Command | Expected result |
| --- | --- | --- |
| Startup | `malvado run --headless startup.lua` | Prints the virtual window sizes, two process cadences and elapsed wall time. |
| Window title | `malvado run title.lua` | Opens a window titled Level 2. |
| Shapes | `malvado run shapes.lua` | Draws filled and outlined shapes, a line and a centered label. |
| Fades | `malvado run --headless fades.lua` | Reaches an opaque blue overlay, then drains the queue back to transparent. |
| Random seed | `malvado run --headless random-seed.lua` | Repeats a roll after restoring the seed. |
| Global import | `malvado run --headless import-globals.lua` | Checks the imported exports against the module values. |
| Legacy sheet | `malvado run legacy-sheet.lua` | Draws a red tile through the legacy frame operation. |
| Cameras | `malvado run --headless cameras.lua` | Checks an explicit move, follow and destruction. |
| Collision | `malvado run --headless collision.lua` | Checks overlap, point containment and process collision. |
| Static world | `malvado run --headless world.lua` | Reports a wall hit and stops the process at 92 pixels. |
| Process control | `malvado run --headless process-control.lua` | Finds attached and detached workers, signals them and ends the level. |
| Input | `malvado run input.lua` | Shows held keys, pointer position, controller state and typed text for 15 seconds. Press Space, click, type or use a controller in slot 0. On Android, tap the Typed label. |
| Text | `malvado run text.lua` | Updates a score label, prints its measured size and removes it. |
| Audio | `malvado run audio.lua` | Plays a short sound followed by a looping track and reports when each stops. |
| Graphics | `malvado run graphics.lua` | Scrolls two process graphs through a fixed clip. |
| Images | `malvado run images.lua` | Draws a full tile and a rotated crop. |
| Atlas | `malvado run atlas.lua` | Prints sprite metadata and draws automatic, manually sampled and static sprites. |
| FPG metadata | `malvado run --headless fpg-metadata.lua` | Checks and prints the metadata for sprites 7 and 42. |
| FPG image | `malvado run fpg-image.lua` | Displays sprite 7 as a 2 × 2 image scaled up for viewing. |

If you use sources from the repository, copy the matching assets into the
run directory as indicated. The ZIPs already contain these files.

| Program | Source asset | Path next to the entry script |
| --- | --- | --- |
| Legacy sheet | [Red tile](../features/image/assets/tile.png) | `tile.png` |
| Images | [Red tile](../features/image/assets/tile.png) | `assets/tile.png` |
| Text | [Anonymous Pro Bold](../../tests/fixtures/fonts/AnonymousPro-Bold.ttf) | `AnonymousPro-Bold.ttf` |
| Audio | [Tone](../audio/tone.wav) | `tone.wav` |
| Atlas | [Aseprite JSON](../aseprite/sheet.json) and [PNG](../aseprite/sheet.png) | `sheet.json` and `sheet.png` |
| FPG metadata and image | [FPG graphics](../fpg/graphics.fpg) | `graphics.fpg` |

Anonymous Pro Bold by Mark Simonson uses the
[SIL Open Font License](../../tests/fixtures/fonts/OFL.txt). The Malvado
artwork, generated fixtures and source code use
[Apache 2.0](../../LICENSE).
