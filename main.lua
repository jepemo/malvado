package.path = package.path .. ";../../?/init.lua"
require 'malvado'

local font = font(60, 0, 0, 0, 255)

Background = process(function(self)
  while not key("escape") do
    love.graphics.setBackgroundColor(231, 231, 231)
    write(font, self.x, self.y, self.text)
    frame()
  end
end)

function love.load()
  set_title("Hello world")
  Background { x=240, y=280, text="Hello World" }
end

-- boilerplate
function love.draw() malvado.draw() end
