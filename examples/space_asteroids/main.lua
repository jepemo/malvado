package.path = package.path .. ";../../?.lua"
require ('malvado')

Stars = process(function(self)
  local screen_width, screen_height = love.graphics.getDimensions()
  stars = {}

  for i=1, self.max_stars do
    local x = love.math.random(5, screen_width-5)
    local y = love.math.random(5, screen_height-5)
    stars[i] = {x, y}
   end

  while not key("escape") do
    love.graphics.setBackgroundColor(0, 0, 0)
    love.graphics.points(stars)
    frame()
  end
end)

SpaceShip = process(function(self)
  local screen_width, screen_height = love.graphics.getDimensions()
  local graph = love.graphics.newImage('spaceship.png')
  self.x = screen_width / 2
  self.y = screen_height / 2
  while not key("escape") do
    love.graphics.draw(graph, self.x, self.y, self.angle, 0.3, 0.3)
    frame()
  end
end)

function love.load()
  set_title('Space Asteroids')
  Stars { z = -1000, max_stars = 200 }
  SpaceShip { z = 0 }
end


-- boilerplate
function love.draw() malvado.draw() end
function love.keypressed(key) malvado.keypressed(key) end
