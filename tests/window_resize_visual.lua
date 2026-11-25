-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
local image, label, music

local Hud = process(function()
  for _ = 1, 30 do
    draw_image {image = image, x = 16, y = 8}
    frame()
  end
end)

local Followed = process(function(self)
  local previous_width, previous_height
  for step = 1, 30 do
    if previous_width then
      local x, y = camera_position()
      assert(x == self.x - previous_width / 2 and y == self.y - previous_height / 2)
    end
    if step == 8 then
      assert(not pcall(set_window_size, -1, 160))
      local w, h = window_size()
      assert(w == 200 and h == 140)
      mv.set_window_size(220, 160)
      mv.set_window_size(220, 160)
    end
    local width, height = window_size()
    assert(width == 200 and height == 140 or width == 220 and height == 160 or width == 240 and height == 180)
    assert(self.x == 300 and self.y == 250 and self.fps == 60)
    local mouse_x, mouse_y = mouse_position()
    assert(type(mouse_x) == "number" and type(mouse_y) == "number")
    if step == 30 then
      assert(width == 240 and height == 180 and music_playing(music))
      change_text(label, {text = "Resized"})
      stop_music(music)
    end
    previous_width, previous_height = width, height
    frame()
  end
end)

return {width = 160, height = 120, title = "Lua window resize", init = function()
  local width, height = mv.window_size()
  assert(width == 160 and height == 120)
  set_window_size(200, 140)
  width, height = window_size()
  assert(width == 200 and height == 140)
  image = load_image("fixtures/red.png")
  label = write {text = "Resize", font = font {size = 12}, x = 8, y = 30}
  music = load_music("fixtures/silence.wav", {loop = true})
  play_music(music)
  Hud {camera = false, region = region {x = 16, y = 8, width = 4, height = 8}}
  camera_follow(Followed {x = 300, y = 250, fps = 60, graph = circle {radius = 5, color = GREEN}})
end}
