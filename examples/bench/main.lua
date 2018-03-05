package.path = package.path .. ";./?/init.lua"
require 'malvado'

local exit_bench = false
local color_fpg = fpg("assets/colours")
local font_texts = font(20)

BenchProcess = process(function(self)
  self.fpg = color_fpg
  self.fpgIndex = 1

  local margin = 50

  self.x = rand(margin, get_screen_width()-margin)
  self.y = rand(margin, get_screen_height()-margin)

  while not exit_bench do
    self.fpgIndex = self.fpgIndex + 1
    frame()
  end
end)

Benchmark = process(function(self)
  write(font_texts, (get_screen_width() / 2)-60, get_screen_height() / 2, "Press spacebar")

  local num_procs = 0
  local start = 0
  local millis_frames = 0

  local tp = write(font_texts, 0, 0, ("Num. Procs: " .. tostring(num_procs)))
  local tm = write(font_texts, get_screen_width() - 350, 0, ("Millis frames: " .. millis_frames))

  while not key("escape") do
    millis_frames = love.timer.getTime() - start

    change_text(tp, "Num. Procs: " .. tostring(num_procs))
    change_text(tm, "Millis frames: " .. millis_frames)

    start = love.timer.getTime()

    if key("space") then
      BenchProcess {}
      num_procs = num_procs + 1
    end

    frame()
  end

  exit_bench = true
end)

-- Start
malvado.start(function()
  set_title('Benchmark')
  Benchmark {}
end, true)
