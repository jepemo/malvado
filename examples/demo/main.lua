package.path = package.path .. ";./?/init.lua"
require 'malvado'

Circle = process(function(self)
  local r = get_screen_height() / 5
  local x = get_screen_width() / 2
  local y = get_screen_height() / 2
  local entra = 0

  while not key("escape") do
    set_color(255, 128, 0)
    draw_fcircle(x, y, r)

    if key("space") and entra == 0 then
      entra = 1
      fade_off()
      fade_on()
    end

    if not key("space") then entra = 0 end

    frame()
  end
end)

function love.load()
  set_title("Fade example")
  Circle { z = 0}
end

-- boilerplate
function love.update(dt) malvado.update(dt) end
