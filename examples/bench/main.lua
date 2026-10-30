-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local images = {}
local stopping = false

local BenchProcess = process(function(self)
  local image_index = 1
  while not stopping do
    draw_image(images[image_index], self.x, self.y)
    image_index = image_index % #images + 1
    frame()
  end
end)

local Benchmark = process(function()
  local font = font(20)
  local prompt = write(font, 300, 300, "Hold Space to add processes")
  local count_text = write(font, 10, 10, "Processes: 0")
  local time_text = write(font, 430, 10, "Tick interval: 0 ms")
  local count = 0
  local previous = time_seconds()

  while not key("escape") do
    local now = time_seconds()
    change_text(time_text, string.format("Tick interval: %.2f ms", (now - previous) * 1000))
    previous = now
    if key("space") then
      BenchProcess { x = math.random(50, 750), y = math.random(50, 550) }
      count = count + 1
      change_text(count_text, "Processes: " .. count)
    end
    frame()
  end

  print("benchmark stopped: " .. count .. " processes")
  stopping = true
  delete_text(prompt)
  delete_text(count_text)
  delete_text(time_text)
end)

start(function()
  math.randomseed(1)
  for i = 0, 8 do
    images[i + 1] = load_image(string.format("examples/bench/assets/colours/%03d.png", i))
  end
  clear_screen(0, 0, 0)
  Benchmark()
end, { width = 800, height = 600, title = "Benchmark" })
