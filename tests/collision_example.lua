-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local real_mv = require("malvado")
local tick = 0
local messages = {}
local circle_x
local square_draws = 0
local tasks = {}
local mv = {}
function mv.import_globals()
  for name, value in pairs(mv) do _G[name] = value end
end
function mv.process(fn)
  return function() tasks[#tasks + 1] = coroutine.create(fn) end
end
function mv.start(init)
  init()
  while tick < 140 do
    for _, task in ipairs(tasks) do
      local ok, err = coroutine.resume(task)
      assert(ok, err)
    end
    tick = tick + 1
  end
  assert(#tasks == 2)
end
function mv.clear_screen() end
function mv.set_color() end
function mv.draw_box() square_draws = square_draws + 1 end
function mv.draw_fcircle(x) circle_x = x end
function mv.draw_text(text)
  if text == "Collision!" then messages[tick] = true end
end
function mv.key(name)
  if name == "escape" then return tick >= 140 end
  if name == "right" then return tick < 75 end
  if name == "left" then return tick >= 75 end
  return false
end
function mv.frame() coroutine.yield() end
package.loaded.malvado = mv
dofile("examples/collision/main.lua")
assert(messages[63] == nil)
assert(messages[64] == true) -- Circle touches the square at x = 380.
assert(messages[75] == true)
assert(messages[84] == true)
assert(messages[85] == nil)
assert(circle_x == 160)
assert(square_draws == 140)
package.loaded.malvado = real_mv
real_mv.start(function() end)
