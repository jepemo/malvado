package.path = package.path .. ";../../?.lua"
require ('malvado')

local font = love.graphics.newFont(60)

Background = process(function(x, y, text)
  while not key("escape") do
    love.graphics.setBackgroundColor(231, 231, 231)
    write(font, x, y, text)
    frame()
  end
end)

function love.load()
  hw = Background(240, 280, "Hello World")
end

function love.draw()
  malvado.draw()
end

function love.keypressed(key)
  malvado.keypressed(key)
end
