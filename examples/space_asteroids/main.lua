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
    clear_screen()
    set_color(255, 255, 255)
    love.graphics.points(stars)
    frame()
  end
end)

Laser = process(function(self)
  self.graph = load_image('laser.png')
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


Asteroid = process(function(self)
  self.graph = load_image('asteroid.png')
  self.x = get_screen_width() / 5
  self.y = get_screen_height() / 5
  self.size = 0.5

  local destroyed = false

  while not key("escape") do
    self.angle = self.angle + 1
    self.x = self.x + self.xdir
    self.y = self.y + self.ydir

    if (self.x < 0)  then self.x = get_screen_width() end
    if (self.x > get_screen_width()) then self.x = 0 end
    if (self.y < 0) then self.y = get_screen_height() end
    if (self.y > get_screen_height()) then self.y = 0 end

    frame()
  end
end)

SpaceShip = process(function(self)
  self.graph = load_image('spaceship.png')
  self.x = get_screen_width() / 2
  self.y = get_screen_height() / 2
  self.size = 0.1
  self.fps = 30
  local angleVelocity = 10
  local vel = 200
  local angleDesp = -90
  local fired = 0

  while not key("escape") do
    if key("left")  then self.angle = self.angle-angleVelocity end
    if key("right") then self.angle = self.angle+angleVelocity end
    if key("up") then
      self.x = self.x+(cos(self.angle+angleDesp) * self.delta * vel)
      self.y = self.y+(sin(self.angle+angleDesp) * self.delta * vel )
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

-- Start
malvado.start(function()
  set_title('Space Asteroids')
  Stars { z = -1, max_stars = 200 }
  SpaceShip { z = 0 }
  Asteroid { z = 1, xdir = 0.2, ydir = 0.8 }
  Asteroid { z = 1, xdir = 0.6, ydir = 0.3 }
end, true)
