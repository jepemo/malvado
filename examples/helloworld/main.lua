package.path = package.path .. ";../../?.lua"
require ('malvado')

local font = love.graphics.newFont(60)

Background = process(function(x, y, text)
  while not key("escape") do
    love.graphics.setBackgroundColor(231, 231, 231)
    love.graphics.setFont(font)
    love.graphics.setColor(0, 0, 0, 255)
    love.graphics.print(text, x, y)
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
