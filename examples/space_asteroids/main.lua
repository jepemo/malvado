-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

require("malvado").import_globals()
local width, height = 800, 600
local images, stars = {}, {}
local game

local function wrap(value, limit)
  return (value + limit) % limit
end

local function hit(a, b, radius)
  local dx, dy = a.x - b.x, a.y - b.y
  return dx * dx + dy * dy < radius * radius
end

local function sprite(image, x, y, scale, angle)
  local w, h = image_size(image)
  draw_image_region(image, x, y, 0, 0, w, h, w / 2, h / 2, scale, angle)
end

local function new_game()
  game = {
    ship = { x = width / 2, y = height / 2, angle = 0, alive = true },
    asteroids = {}, lasers = {}, score = 0, firing = false
  }
  for i = 1, 4 do
    game.asteroids[i] = {
      x = 80 + i * 130, y = 80 + i * 30,
      dx = i % 2 == 0 and 1.2 or -1.2, dy = 0.5, angle = i * 30
    }
  end
end

local function fire()
  local ship = game.ship
  local angle = math.rad(ship.angle - 90)
  game.lasers[#game.lasers + 1] = {
    x = ship.x, y = ship.y,
    dx = math.cos(angle) * 6, dy = math.sin(angle) * 6, life = 100
  }
end

local function update()
  local ship = game.ship
  if ship.alive then
    if key("left") then ship.angle = ship.angle - 4 end
    if key("right") then ship.angle = ship.angle + 4 end
    if key("up") then
      local angle = math.rad(ship.angle - 90)
      ship.x = wrap(ship.x + math.cos(angle) * 3, width)
      ship.y = wrap(ship.y + math.sin(angle) * 3, height)
    end
    local pressed = key("space")
    if pressed and not game.firing then fire() end
    game.firing = pressed
  end

  for i = #game.lasers, 1, -1 do
    local laser = game.lasers[i]
    laser.x, laser.y = laser.x + laser.dx, laser.y + laser.dy
    laser.life = laser.life - 1
    if laser.life <= 0 or laser.x < 0 or laser.x >= width or laser.y < 0 or laser.y >= height then
      table.remove(game.lasers, i)
    else
      for j = #game.asteroids, 1, -1 do
        if hit(laser, game.asteroids[j], 35) then
          table.remove(game.asteroids, j)
          table.remove(game.lasers, i)
          game.score = game.score + 1
          break
        end
      end
    end
  end

  for _, asteroid in ipairs(game.asteroids) do
    asteroid.x = wrap(asteroid.x + asteroid.dx, width)
    asteroid.y = wrap(asteroid.y + asteroid.dy, height)
    asteroid.angle = asteroid.angle + 1
    if ship.alive and hit(ship, asteroid, 32) then ship.alive = false end
  end
end

local function draw()
  for _, star in ipairs(stars) do draw_text(".", star.x, star.y, 10) end
  for _, asteroid in ipairs(game.asteroids) do
    sprite(images.asteroid, asteroid.x, asteroid.y, 0.3, asteroid.angle)
  end
  for _, laser in ipairs(game.lasers) do sprite(images.laser, laser.x, laser.y, 1, 0) end
  if game.ship.alive then sprite(images.ship, game.ship.x, game.ship.y, 0.1, game.ship.angle) end
  draw_text("Score: " .. game.score, 10, 10, 20)
  if not game.ship.alive then
    draw_text("Game over - press R to restart", 220, 270, 24)
  elseif #game.asteroids == 0 then
    draw_text("You win - press R to restart", 220, 270, 24)
  end
end

local Game = process(function()
  local restart_down = false
  new_game()
  while not key("escape") do
    update()
    local restart = key("r")
    if restart and not restart_down and (not game.ship.alive or #game.asteroids == 0) then new_game() end
    restart_down = restart
    draw()
    frame()
  end
end)

start(function()
  math.randomseed(1)
  for i = 1, 100 do stars[i] = { x = math.random(5, width - 5), y = math.random(5, height - 5) } end
  images.ship = load_image("examples/space_asteroids/spaceship.png")
  images.asteroid = load_image("examples/space_asteroids/asteroid.png")
  images.laser = load_image("examples/space_asteroids/laser.png")
  clear_screen(0, 0, 0)
  Game()
end, { width = width, height = height, title = "Space Asteroids" })
