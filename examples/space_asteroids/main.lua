-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()

local W, H = 800, 600
local score, result = 0, nil

-- Asteroids

local Rock = process(function(self)
  self.graph = load_asset("examples/space_asteroids/asteroid.png", {pivot = "center"})

  while true do
    self.x = (self.x + self.vx) % W
    self.y = (self.y + self.vy) % H
    self.angle = self.angle + 1
    frame()
  end
end, {defaults = {collider = mv.collision.circle(96)}})

-- Lasers

local Shot = process(function(self)
  self.graph = load_asset("examples/space_asteroids/laser.png", {pivot = "center"})

  for _ = 1, 90 do
    self.x = self.x + self.vx
    self.y = self.y + self.vy

    if self.x < 0 or self.x >= W or self.y < 0 or self.y >= H then return end

    local rock = collision(Rock)
    if rock then
      if rock.size > 0.15 then
        for i = 0, 2 do
          local angle = math.rad(i * 120)
          spawn(Rock, {
            x = rock.x, y = rock.y,
            size = rock.size / 2,
            vx = math.cos(angle) * 2,
            vy = math.sin(angle) * 2
          }, {attached = false})
        end
      end

      kill(rock)
      score = score + 1
      return
    end

    frame()
  end
end, {defaults = {collider = mv.collision.circle(3)}})

-- Spaceship

local Ship = process(function(self)
  self.graph = load_asset("examples/space_asteroids/spaceship.png", {pivot = "center"})
  local firing = false

  while not result do
    if key("left") then self.angle = self.angle - 4 end
    if key("right") then self.angle = self.angle + 4 end

    local angle = math.rad(self.angle - 90)
    local dx, dy = math.cos(angle), math.sin(angle)

    if key("up") then
      self.x = (self.x + dx * 3) % W
      self.y = (self.y + dy * 3) % H
    end

    local pressed = key("space")
    if pressed and not firing then
      Shot {x = self.x, y = self.y, vx = dx * 7, vy = dy * 7}
    end

    firing = pressed

    if collision(Rock) then result = "Game over!" end
    frame()
  end
end, {defaults = {size = 0.1, collider = mv.collision.circle(110, {offset_y = 10})}})

-- Stars

local Star = process(function(self)
  self.graph = circle(1)
  self.z = -10
  while true do frame() end
end)

-- Game

local Game = process(function(self)
  self.camera = false
  self.z = 10
  math.randomseed(42)

  for i = 1, 40 do
    Star {x = math.random(W), y = math.random(H)}
  end

  Ship {x = W / 2, y = H / 2}

  for i = 1, 5 do
    Rock {
      x = 80 + i * 130, y = 80 + i * 30,
      size = 0.3,
      vx = i % 2 == 0 and 1.2 or -1.2,
      vy = 0.5
    }
  end

  while not key("escape") do
    if score == 20 then result = "You win!" end

    draw_text("Score: " .. score, 20, 20, 24)
    if result then draw_text(result, 300, 280, 28) end
    frame()
  end

  -- Fragments are independent so they survive the shot that creates them.
  for _, rock in ipairs(find(Rock)) do kill(rock) end
  kill_tree(self)
end)

return {
  title = "Space Asteroids",
  width = W,
  height = H,
  init = function()
    clear_screen(8, 10, 24)
    Game()
  end
}
