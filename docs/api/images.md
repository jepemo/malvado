# Draw a PNG image

You can load a PNG image after you open a window. The image uses its original size. Its position is the top-left corner, in window pixels.

## Lua

Load the image in the function you pass to `start`. Draw it inside a process, before each `frame`:

```lua
local mv = require("malvado")
local image

local picture = mv.process(function()
    while not mv.key("escape") do
        mv.draw_image(image, 40, 40)
        mv.frame()
    end
end)

mv.start(function()
    image = mv.load_image("picture.png")
    local width, height = mv.image_size(image)
    print("Image size:", width, height)
    picture()
end)
```

Paths are relative to the current working directory. `load_image` reports an error if the file is not a valid PNG or there is no window. Images are not available in headless mode. Lua keeps an image alive while a draw command uses it. It releases the image when it is no longer used or when the runner closes.

## C

Include `malvado/platform.h` and link the raylib platform library. Open a window before loading an image. Check the result, and destroy the image before you close the window:

```c
MvImage *image = mv_image_load("picture.png");
if (image) {
    mv_window_begin((MvColor){0, 0, 0, 255});
    mv_image_draw(image, 40, 40);
    mv_window_end();
    mv_image_destroy(image);
}
```

`mv_image_load` returns `NULL` if loading fails or the window is not open. `mv_image_width` and `mv_image_height` return the dimensions in pixels, or zero for `NULL`. `mv_image_destroy(NULL)` is safe. Use the window and image functions on the window thread.

This first image API draws whole PNG files. Atlas metadata, regions, pivots, and transforms are still planned for alpha.3.
