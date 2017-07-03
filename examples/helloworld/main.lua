package.path = package.path .. ";../../?.lua"
require ('malvado')

local font = font(60, 0, 0, 0, 255)

Background = process(function(self)
  while not key("escape") do
    love.graphics.setBackgroundColor(231, 231, 231)
    write(font, self.x, self.y, self.text)
    frame()
  end
end)

function love.load()
  hw = Background { x=240, y=280, text="Hello World" }
end

function love.draw()
  malvado.draw()
end

function love.keypressed(key)
  malvado.keypressed(key)
end
