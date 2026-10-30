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
