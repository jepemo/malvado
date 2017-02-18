package.path = package.path .. ";../../?.lua"
local Base  = require ('malvado')

-- background = process({
--
-- }, function()
--   love.graphics.print("Hello World", 400, 300)
-- end)

-- function love.load()
--   hw = background("text")
-- end

function love.draw()
  -- hw:draw()
  print "1"
end

function love.update()
  -- hw:update()
  print "2"
end
