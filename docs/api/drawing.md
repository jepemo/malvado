# Draw rectangles and circles

A process can draw filled or outlined shapes before it calls `mv.frame()`. Coordinates and radii use integer window pixels. The origin is at the top-left corner.

```lua
local mv = require("malvado")
local Shape = mv.process(function()
  while not mv.key("escape") do
    mv.set_color(60, 160, 255)
    mv.draw_box(20, 20, 120, 80)
    mv.set_color(255, 255, 255, 180)
    mv.draw_circle(180, 50, 30)
    mv.frame()
  end
end)
mv.start(function() Shape() end)
```

`set_color(r, g, b, a)` sets the color for later primitive commands. Alpha defaults to 255. Each channel is an integer from 0 to 255. The color is shared by processes, so set it before drawing when a particular color is needed. A queued command keeps the color it had when it was created. The initial color is opaque white.

- `mv.draw_box(x1, y1, x2, y2)` draws a filled rectangle. `mv.draw_rect` draws its outline. The second corner must be below and to the right of the first.
- `mv.draw_fcircle(x, y, radius)` draws a filled circle. `mv.draw_circle` draws its outline. The radius must be positive.

Invalid geometry raises an error. Draw commands are valid only inside a running process. They follow the process's `z` order and persist until its next activation. The legacy compatibility import also provides these functions as globals.

In C, call `mv_window_draw_box(x, y, width, height, filled, color)` or `mv_window_draw_circle(x, y, radius, filled, color)` between `mv_window_begin` and `mv_window_end`. Pass 1 for filled or 0 for outlined. The functions return 0 for invalid geometry or no window. C drawing follows call order.

## Fade the screen

`mv.fade_off(ticks)` fades to opaque black, and `mv.fade_on(ticks)` fades back to transparent. The duration defaults to 16 simulation ticks. Calls are queued, so `mv.fade_off(); mv.fade_on()` runs the two transitions in order. The legacy import also supplies these two names. Its argument is a duration in ticks; it does not reproduce the historical speed argument.

For another color, call `mv.fade_to(r, g, b, a, ticks)`. Channels are integers from 0 to 255; duration is an integer from 0 to 1,000,000 ticks. A duration of zero changes the color immediately when the request reaches the front of the queue. Up to 16 requests can wait behind the current transition; adding another raises an error. `mv.fade_active()` is true while a transition or queued request remains. `mv.fade_color()` returns the current red, green, blue, and alpha values. The overlay is drawn after processes and persistent text. It advances once per simulation tick and stops when the runner stops.

In C, initialize `MvFade fade = {0}` and call `mv_fade_start(&fade, target, ticks)`. Call `mv_fade_step(&fade)` on each simulation tick and use `fade.current` as the color of a window-sized filled box. `mv_fade_active(&fade)` reports whether the transition is in progress. A start with invalid input returns 0 without changing the state. C callers manage any transition queue themselves.
