package.path = package.path .. ";../../?.lua"
require ('malvado')

Background = process(function(x, y, text, font)
  while not key("escape") do
    love.graphics.setBackgroundColor(231, 231, 231)
    love.graphics.setFont(font)
    love.graphics.print(text, x, y)
    frame()
  end
end)

function love.load()
  local font = love.graphics.newFont(60)
  hw = Background(250, 280, "Hello World", font)
end

function love.draw()
  malvado.draw()
end

function love.keypressed(key)
  malvado.keypressed(key)
end
