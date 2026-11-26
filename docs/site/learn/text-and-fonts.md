# Text and fonts

Build a scoreboard with a centered score that changes only when you earn
a point. Draw a second line to show the player which key to press.

## Run the example

Use a desktop SDK that includes text alignment and custom fonts.
These features landed after 2.1.0. If you use 2.1.0, follow the
[local development SDK instructions](../../contributing/build/#install-a-local-development-build).
Download `labels.lua` and open a terminal in the download directory.

```sh
malvado run labels.lua
```

Press Space to add a point. The score stays centered as it gains digits.
Press Escape or close the window to exit. The example uses the built-in
font and needs no font file.

<!-- example -->

## Keep a label or draw it again

`write` creates persistent text and gives you a numeric ID. Keep the ID
and pass it to `change_text` to update only the fields you supply. Creating
a new label on every turn leaves old labels in the registry until you
delete them or their owning process ends. Persistent text draws in screen
space. Use `delete_text` to remove a label early.

`draw_text` queues text for the current process, like an immediate shape.
Call it again on each activation that should show the prompt. It follows
that process's camera, clipping and draw order. Use `self.camera = false`
for a screen-space prompt in a scrolling game.

## Share a font and align the text

Both paths accept the same font descriptor and alignment constants.
`TEXT_ALIGN_CENTER` centers the measured text about x. With
`TEXT_ALIGN_LEFT`, x is the left edge. With `TEXT_ALIGN_RIGHT`, it is the
right edge. Alignment leaves y unchanged.

The example uses `mv.font {size = 24}`. For a custom face, put your licensed
TTF or OTF in the [asset directory](resources.md) and use
`mv.load_font {path = "my-font.ttf", size = 24}` instead. Use
`text_size(face, text)` to measure the width and height in pixels when
calculating a layout.

Try using `TEXT_ALIGN_RIGHT` for the score and keeping its x at 160.
Its right edge stays in place as you add points.

See the [Lua text reference](../../reference/lua/text/), including
[immediate text drawing](../../reference/lua/drawing/draw_text/). The
[C and Lua text guide](../../concepts/text/) and [experimental Rust font
guide](../../concepts/rust-font/) cover the other bindings.
