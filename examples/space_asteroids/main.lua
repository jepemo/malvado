package.path = package.path .. ";../../?.lua"
require ('malvado')

Stars = process(function(self)
  stars = {}
  for i=1, self.max_stars do
    local x = love.math.random(5, get_screen_width()-5)
    local y = love.math.random(5, get_screen_height()-5)
    stars[i] = {x, y}
   end

  while not key("escape") do
    love.graphics.setBackgroundColor(0, 0, 0)
    love.graphics.points(stars)
    frame()
  end
end)

SpaceShip = process(function(self)
  self.graph = love.graphics.newImage('spaceship.png')
  self.x = get_screen_width() / 2
  self.y = get_screen_height() / 2
  self.size = 0.1
  local angleVelocity = 1
  local angleInit = -90

  while not key("escape") do
    if key("left")  then self.angle = self.angle-angleVelocity end
    if key("right") then self.angle = self.angle+angleVelocity end
    if key("up") then
      self.x = self.x+cos(self.angle+angleInit)
      self.y = self.y+sin(self.angle+angleInit)
    end
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
