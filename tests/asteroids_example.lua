-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local process, key = mv.process, mv.key
local interactive = os.getenv("MALVADO_REAL_INPUT") == "1"
local controls, updates = {}, 0

local function upvalue(fn, name)
  for i = 1, math.huge do
    local found, value = debug.getupvalue(fn, i)
    if not found then break end
    if found == name then return i, value end
  end
  error("missing test upvalue: " .. name)
end

if interactive then
  mv.key = function(name)
    local down = key(name)
    if down then controls[name] = true end
    return down
  end
else
  mv.key = function(name) return name == "escape" and updates >= 2 end
  mv.process = function(fn)
    local index, update = upvalue(fn, "update")
    local _, new_game = upvalue(fn, "new_game")
    local game_index = upvalue(update, "game")
    local _, fire = upvalue(update, "fire")
    debug.setupvalue(fn, index, function()
      local _, game = debug.getupvalue(update, game_index)
      updates = updates + 1
      if updates == 1 then
        game.asteroids = {{x = game.ship.x, y = game.ship.y - 6, dx = 0, dy = 0, angle = 0}}
        fire()
        update()
        assert(game.score == 1 and #game.asteroids == 0 and game.ship.alive)
        game.asteroids = {{x = game.ship.x, y = game.ship.y, dx = 0, dy = 0, angle = 0}}
      else
        update()
        assert(not game.ship.alive)
        new_game()
        local _, restarted = debug.getupvalue(update, game_index)
        assert(restarted.ship.alive and restarted.score == 0 and #restarted.asteroids == 4)
      end
    end)
    return process(fn)
  end
end

dofile("examples/space_asteroids/main.lua")
if interactive then
  assert(controls.left and controls.up and controls.space)
else
  assert(updates == 2)
end
