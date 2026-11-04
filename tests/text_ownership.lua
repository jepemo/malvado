-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local game_text, process_text, deleted
local owner = mv.process(function()
  process_text = mv.write(mv.font(16), 0, 0, "owned")
  deleted = mv.write(mv.font(16), 0, 0, "removed")
  mv.delete_text(deleted)
end)
local killed_text
local killed = mv.process(function()
  killed_text = mv.write(mv.font(16), 0, 0, "killed")
  while true do mv.frame() end
end)
local observer = mv.process(function()
  mv.frame()
  assert(not pcall(mv.change_text, process_text, "gone"))
  assert(not pcall(mv.delete_text, process_text))
  assert(not pcall(mv.delete_text, deleted))
  mv.change_text(game_text, "still here")
  mv.kill(killed)
  mv.frame()
  assert(not pcall(mv.change_text, killed_text, "gone"))
  mv.delete_text(game_text)
end)
mv.start(function()
  game_text = mv.write(mv.font(16), 0, 0, "game")
  owner()
  killed = killed()
  observer()
end)
