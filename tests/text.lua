-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
assert(not pcall(mv.load_font, {path = "fixtures/fonts/AnonymousPro-Bold.ttf", size = 24}))
assert(not pcall(mv.load_font, {path = "fixtures/fonts/AnonymousPro-Bold.ttf", size = 0}))
assert(not pcall(mv.load_font, {path = "fixtures/fonts/AnonymousPro-Bold.ttf", size = 24, extra_glyphs = "\0"}))
local id
local task = mv.process(function()
  local f = mv.font {size = 24}
  assert(f.size == 24)
  assert(not pcall(mv.font, {size = 0}))
  assert(not pcall(mv.font, {size = 1025}))
  assert(not pcall(mv.font, {size = "big"}))
  assert(not pcall(mv.font, {wrong = 1}))
  assert(not pcall(mv.write, {font = f, text = "X", x = 0, y = 0, align = "invalid"}))
  assert(not pcall(mv.write, {font = f, text = "X", x = 0, y = 0, align = "left\0right"}))
  assert(not pcall(mv.write, {font = f, text = "X", x = 0, y = 0, unknown = 1}))
  assert(not pcall(mv.write, {font = {size = "x"}, text = "X", x = 0, y = 0}))
  assert(not pcall(mv.write, {font = {size = 24, resource = {}}, text = "X", x = 0, y = 0}))
  assert(not pcall(mv.write, {font = f, text = "X", x = 0, y = 0, size = 0}))
  for _, value in ipairs({0x100000000, -0x100000000, math.mininteger, math.maxinteger}) do
    assert(not pcall(mv.write, {font = f, text = "X", x = value, y = 0}))
    assert(not pcall(mv.write, {font = f, text = "X", x = 0, y = value}))
  end
  local boundary = mv.write {font = f, text = "X", x = -2147483648, y = 2147483647}
  assert(not pcall(mv.write, {font = f, text = "A\0B", x = 0, y = 0}))
  assert(not pcall(mv.write, {font = f, text = "X", x = 0, y = 0, color = {255, 255}}))
  assert(not pcall(mv.write, {font = f, text = "X", x = 0, y = 0, color = {255, 255, 255, 256}}))
  assert(not pcall(mv.write, {font = f, text = "X", x = 0, y = 0, color = {255, -1, 255, 255}}))
  mv.delete_text(boundary)
  id = mv.write {font = f, text = "Hola", x = 4, y = 5}
  assert(id == boundary + 1)
  -- Per-text styles are independent while the font stays shared.
  local red = mv.write {font = f, text = "red", x = 0, y = 0, color = {255, 0, 0, 255}}
  local blue = mv.write {font = f, text = "blue", x = 0, y = 30, size = 40, color = {0, 0, 255, 128}}
  mv.change_text(red, {color = {0, 255, 0, 200}})
  assert(not pcall(mv.change_text, blue, {color = {0, 0, 255, 300}}))
  -- Colors are copied, so mutating the source table changes nothing.
  local shared = {13, 59, 102, 255}
  local copied = mv.write {font = f, text = "copy", x = 0, y = 0, color = shared}
  shared[1] = 250
  mv.change_text(copied, {color = shared})
  shared[4] = 0
  mv.delete_text(copied)
  -- Partial updates keep every other property unchanged.
  assert(not pcall(mv.change_text, id, {text = "bad\0text"}))
  assert(not pcall(mv.change_text, id, {x = 0x100000000}))
  assert(not pcall(mv.change_text, id, {y = math.maxinteger}))
  assert(not pcall(mv.change_text, id, {align = "bogus"}))
  assert(not pcall(mv.change_text, id, {size = 0}))
  assert(not pcall(mv.change_text, id, {color = {1, 2}}))
  assert(not pcall(mv.change_text, id, {font = {size = 0}}))
  assert(not pcall(mv.change_text, id, {bogus = 1}))
  mv.change_text(id, {text = "Adiós"})
  mv.change_text(id, {x = 20})
  mv.change_text(id, {align = "right"})
  mv.change_text(id, {text = "Moved", x = 20, y = 30, font = mv.font {size = 18}})
  mv.change_text(id, {size = 60})
  mv.change_text(id, {color = {255, 0, 0, 180}})
  mv.change_text(id, {align = "center"})
  mv.frame()
  collectgarbage("collect")
  mv.change_text(id, {text = "Still here"})
  -- A failing option must not apply a partial update.
  assert(not pcall(mv.change_text, id, {text = "changed", color = {0, 0, 0, 999}}))
  mv.delete_text(red)
  mv.delete_text(blue)
  mv.delete_text(id)
  assert(not pcall(mv.delete_text, id))
  assert(not pcall(mv.change_text, id, {text = "X"}))
  local width, height = mv.text_size(f, "Hola")
  assert(width == 0 and height == 0) -- Headless has no font metrics.
  assert(not pcall(mv.text_size, f))
  assert(not pcall(mv.text_size, f, "bad\0text"))
  assert(not pcall(mv.text_size, {size = 0}, "X"))
  assert(not pcall(mv.text_size, 999999))
  local owned
  local child = mv.process(function()
    owned = mv.write {font = f, text = "owned", x = 0, y = 0}
    while true do mv.frame() end
  end)
  local handle = child()
  -- The parent runs before newly spawned children in creation order.
  mv.frame()
  mv.frame()
  assert(owned)
  mv.kill(handle)
  mv.frame()
  collectgarbage("collect")
  assert(not pcall(mv.change_text, owned, {text = "dead"}))
  local returned
  mv.process(function() returned = mv.write {font = f, text = "return", x = 0, y = 0} end)()
  mv.frame()
  mv.frame()
  assert(returned)
  assert(not pcall(mv.delete_text, returned))
end)
mv.start(function() task {} end)
assert(id ~= nil)
