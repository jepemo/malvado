-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local images = {}
local stopping = false
local smoke = os.getenv("MALVADO_SMOKE") == "1"
local worker_ticks = 0

local BenchProcess = mv.process(function(self)
  local frame = 1
  while not stopping do
    mv.draw_image(images[frame], self.x, self.y)
    worker_ticks = worker_ticks + 1
    frame = frame % #images + 1
    mv.frame()
  end
end)

local Benchmark = mv.process(function()
  local font = mv.font(20)
  local prompt = mv.write(font, 300, 300, "Hold Space to add processes")
  local count_text = mv.write(font, 10, 10, "Processes: 0")
  local time_text = mv.write(font, 430, 10, "Tick interval: 0 ms")
  local count = 0
  local ticks = 0
  local previous = mv.time_seconds()

  while not mv.key("escape") do
    local now = mv.time_seconds()
    mv.change_text(time_text, string.format("Tick interval: %.2f ms", (now - previous) * 1000))
    previous = now
    if mv.key("space") or (smoke and ticks < 3) then
      BenchProcess { x = math.random(50, 750), y = math.random(50, 550) }
      count = count + 1
      mv.change_text(count_text, "Processes: " .. count)
    end
    ticks = ticks + 1
    if smoke and ticks == 5 then
      assert(count == 3 and worker_ticks > 0)
      print("benchmark smoke: 3 processes ran")
      break
    end
    mv.frame()
  end

  print("benchmark stopped: " .. count .. " processes")
  stopping = true
  mv.delete_text(prompt)
  mv.delete_text(count_text)
  mv.delete_text(time_text)
end)

mv.start(function()
  math.randomseed(1)
  for i = 0, 8 do
    images[i + 1] = mv.load_image(string.format("examples/bench/assets/colours/%03d.png", i))
  end
  mv.clear_screen(0, 0, 0)
  Benchmark()
end, { width = 800, height = 600, title = "Benchmark" })
