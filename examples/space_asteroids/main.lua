package.path = package.path .. ";./?/init.lua"
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

Laser = process(function(self)
  self.graph = love.graphics.newImage('laser.png')
  self.size = 0.5
  local velocity = 3

  while not key("escape")
    and self.x > 0 and self.x < get_screen_width()
    and self.y > 0 and self.y < get_screen_height() do
    self.x = self.x + self.xdir * velocity
    self.y = self.y + self.ydir * velocity
    frame()
  end
end)

SpaceShip = process(function(self)
  self.graph = love.graphics.newImage('spaceship.png')
  self.x = get_screen_width() / 2
  self.y = get_screen_height() / 2
  self.size = 0.1
  local angleVelocity = 1
  local angleDesp = -90
  local fired = 0

  while not key("escape") do
    if key("left")  then self.angle = self.angle-angleVelocity end
    if key("right") then self.angle = self.angle+angleVelocity end
    if key("up") then
      self.x = self.x+cos(self.angle+angleDesp)
      self.y = self.y+sin(self.angle+angleDesp)
    end

    if key("space") and fired == 0 then
      Laser { x = self.x,
              y = self.y,
              angle = self.angle,
              xdir = cos(self.angle+angleDesp),
              ydir = sin(self.angle+angleDesp)}
      fired = 1
    end

    if not key("space") then fired = 0 end

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
