-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local define, original_key, original_frame, original_text = mv.process, mv.key, mv.frame, mv.draw_text
local interactive = os.getenv("MALVADO_REAL_INPUT") == "1"
local constructors, threads, handles, controls, seen = {}, {}, {}, {}, {}
local definitions, ticks = 0, 0
local target, split_checked, shot_checked, loss_checked
local function assert_radius(actor, radius)
  local inside, outside = mv.world.create(), mv.world.create()
  inside:add_rect({x = actor.x + radius - 0.5, y = actor.y - 0.1, width = 0.2, height = 0.2})
  outside:add_rect({x = actor.x + radius + 0.5, y = actor.y - 0.1, width = 0.2, height = 0.2})
  assert(inside:collision(actor) == 1 and outside:collision(actor) == nil)
end
mv.process = function(fn, options)
  definitions = definitions + 1
  local kind = definitions
  local constructor = define(function(self)
    threads[coroutine.running()] = kind
    handles[kind] = self
    return fn(self)
  end, options)
  constructors[kind] = constructor
  return constructor
end
mv.key = function(name)
  if interactive then
    local pressed = original_key(name)
    if pressed then controls[name] = true end
    return pressed
  end
  if name == "escape" then return ticks >= 20 end
  if name == "space" then return (ticks >= 6 and ticks < 11) or ticks == 12 end
  if name == "left" or name == "up" then return ticks == 6 end
  return false
end
mv.draw_text = function(text, ...)
  seen[text] = true
  return original_text(text, ...)
end
mv.frame = function()
  if not interactive and threads[coroutine.running()] == 5 then
    ticks = ticks + 1
    local Rock, Shot = constructors[1], constructors[2]
    if ticks == 1 then
      local rocks = mv.find(Rock)
      assert(#rocks == 5)
      for _, rock in ipairs(rocks) do
        rock.x, rock.y, rock.vx, rock.vy = 700, 50, 0, 0
      end
      target = rocks[1]
      target.x, target.y = 650, 400
    elseif ticks == 2 then
      -- A shot hits and splits a large rock, then terminates naturally.
      Shot({x = 650, y = 440, vx = 0, vy = -7})
    elseif ticks == 6 then
      assert(seen["Score: 1"] and target.status == "dead")
      local rocks = mv.find(Rock)
      assert(#rocks == 7)
      local fragments = 0
      for _, rock in ipairs(rocks) do
        if rock.size == 0.15 then
          fragments = fragments + 1
          assert_radius(rock, 14.4)
        else assert_radius(rock, 28.8) end
      end
      assert(fragments == 3)
      split_checked = true
    elseif ticks == 10 then
      assert(#mv.find(Shot) == 1) -- Holding Space does not fire each tick.
      assert(handles[3].size == 0.1 and handles[3].x ~= 400 and handles[3].y ~= 300)
      shot_checked = true
    elseif ticks == 14 then
      assert(#mv.find(Shot) == 2) -- A second press creates a second shot.
      local rock = mv.find(Rock)[1]
      rock.x, rock.y, rock.vx, rock.vy = handles[3].x, handles[3].y, 0, 0
    elseif ticks == 17 then
      assert(seen["Game over!"] and #mv.find(constructors[3]) == 0)
      assert(#mv.find(Shot) == 0) -- Ship completion cleans up its owned shots.
      loss_checked = true
    end
  end
  return original_frame()
end
local game = dofile("examples/space_asteroids/main.lua")
mv.start(game.init, {width = game.width, height = game.height, title = game.title})
if interactive then
  assert(controls.left and controls.up and controls.space)
else
  assert(ticks == 20 and split_checked and shot_checked and loss_checked)
end
