-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
mv.import_globals()
assert(TEXT_ALIGN_LEFT == "left" and mv.TEXT_ALIGN_LEFT == TEXT_ALIGN_LEFT)
assert(TEXT_ALIGN_CENTER == "center" and mv.TEXT_ALIGN_CENTER == TEXT_ALIGN_CENTER)
assert(TEXT_ALIGN_RIGHT == "right" and mv.TEXT_ALIGN_RIGHT == TEXT_ALIGN_RIGHT)
local actor = mv.process(function()
  local id = mv.write {font = mv.font {size = 20}, text = "Lua", x = 1, y = 2, align = TEXT_ALIGN_CENTER}
  assert(id == 1)
  for _, invalid in ipairs {
    {align = "middle"}, {align = "left\0"}, {align = 1}, {font = false},
    {font = {}}, {font = {size = 20, resource = {}}}, {size = "20"},
    {font = font {size = 20}, size = 0}, {font = font {size = 20}, size = 1025},
    {size = 2.5}, {text = "bad\0"}, {unknown = true},
  } do
    local options = {text = "invalid", x = 1, y = 2}
    for key, value in pairs(invalid) do options[key] = value end
    assert(not pcall(mv.draw_text, options))
  end
  assert(not pcall(mv.write, {font = font {size = 20}, text = "bad", x = 1, y = 2, align = "middle"}))
  assert(not pcall(mv.change_text, id, {align = "middle", text = "bad"}))
  set_color(RED)
  mv.draw_text {text = "", x = 1, y = 2}
  draw_text {text = "one\ntwo", x = 1, y = 2, align = "center", font = font {size = 24}}
  draw_text {text = "override", x = 1, y = 2, font = font {size = 24}, size = 30, align = TEXT_ALIGN_RIGHT}
  mv.frame()
  change_text(id, {align = TEXT_ALIGN_LEFT})
  change_text(id, {align = "right"})
  mv.change_text(id, {text = "Changed"}) -- Preserve the coordinates/alignment set by C.
  mv.frame()
  assert(not pcall(mv.change_text, id, {text = "removed"}))
  assert(not pcall(mv.delete_text, id))
  collectgarbage("collect")
end)
mv.start(function() actor() end)
