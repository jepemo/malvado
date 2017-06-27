package.path = package.path .. ";../../?.lua"
local malvado  = require ('malvado')

Background = process(function(x, y, text)
  while not key("escape") do
    love.graphics.print(text, x, y)
    coroutine.yield()
  end
end)

function love.load()
  hw = Background(400, 300, "Hello World")
end

function love.update(dt)
  engine.update(dt)
end

function love.keypressed(key)
  engine.keypressed(key)
end
