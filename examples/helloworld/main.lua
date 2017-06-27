package.path = package.path .. ";../../?.lua"
require ('malvado')

Background = process(function(x, y, text)
  while not key("escape") do
    love.graphics.print(text, x, y)
    frame()
  end
end)

function love.load()
  hw = Background(400, 300, "Hello World")
end

function love.update(dt)
  --malvado.update(dt)
end

function love.draw()
  malvado.update(0)
end

function love.keypressed(key)
  malvado.keypressed(key)
end
