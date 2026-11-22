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
  local f = font {size = 20}
  local prompt = write {font = f, text = "Hold Space to add processes", x = 300, y = 300}
  local count_text = write {font = f, text = "Processes: 0", x = 10, y = 10}
  local time_text = write {font = f, text = "Tick interval: 0 ms", x = 430, y = 10}
  local count = 0
  local previous = time_seconds()

  while not key("escape") do
    local now = time_seconds()
    change_text(time_text, {text = string.format("Tick interval: %.2f ms", (now - previous) * 1000)})
    previous = now
    if key("space") then
      BenchProcess { x = math.random(50, 750), y = math.random(50, 550) }
      count = count + 1
      change_text(count_text, {text = "Processes: " .. count})
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
  clear_screen {0, 0, 0, 255}
  Benchmark()
end, { width = 800, height = 600, title = "Benchmark" })
