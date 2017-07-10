package.path = package.path .. ";./?/init.lua"
require 'malvado'

Circle = process(function(self)
  local r = get_screen_height() / 5
  local x = get_screen_width() / 2
  local y = get_screen_height() / 2

  while not key("escape") do
    set_color(255, 128, 0)
    draw_fcircle(x, y, r)

    if key("space") then
      fade_off()
    end

    frame()
  end
end)

function love.load()
  set_title("Fade example")
  Circle { z = 0}
end

-- boilerplate
function love.draw() malvado.draw() end
