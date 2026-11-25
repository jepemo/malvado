-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local proc = mv.process
assert(kill == nil)

local Actor = proc(function()
  while true do mv.frame() end
end)
local shape = mv.collision.circle {radius = 2}
local function same_list(a, b)
  assert(#a == #b)
  for i, entry in ipairs(a) do assert(b[i] == entry) end
end

local Controller = proc(function(self)
  local a = Actor {collider = shape, send = false}
  local b = Actor {collider = shape}
  local loose = Actor({}, {attached = false})
  assert(a.send == false and a.status == "running" and a.parent == self)
  assert(not pcall(function() a.status = "dead" end))
  local function custom_kill(target) return target.id end
  a.kill = custom_kill
  assert(a:kill() == a.id)
  a.kill = nil
  assert(a.kill == b.kill)
  a.send = nil
  assert(a.send == b.send)
  assert(not pcall(b.alive, a.id) and not pcall(b.kill, nil))
  assert(getmetatable(a) == "malvado process")
  assert(a:alive() == proc.alive(a))
  assert(a.alive(a) and not pcall(a.alive, nil))
  assert(select("#", a:set_priority(5)) == 0)
  assert(select("#", proc.set_priority(b, 5)) == 0)
  assert(a.priority == b.priority)
  a.set_priority(a, 6)
  assert(a.priority == 6 and not pcall(a.set_priority, 9))

  assert(select("#", a:signal(mv.SIGNAL_SLEEP)) == 0)
  assert(select("#", proc.signal(b, mv.SIGNAL_SLEEP)) == 0)
  assert(a.status == b.status and a.status == "sleeping")
  a.signal(a, mv.SIGNAL_WAKEUP)
  assert(a.status == "running" and not pcall(a.signal, a, "bad signal"))
  local creation = {relation = "creation"}
  same_list(self:children(), proc.children(self))
  same_list(self.children(self, creation), proc.children(self, creation))
  same_list(self:descendants(creation), proc.descendants(self, creation))
  same_list(self.descendants(self), proc.descendants(self))
  assert(#self:children(creation) == 3 and #self:children() == 2)
  assert(not pcall(self.children, self, {relation = "bad"}))
  assert(self:signal_tree(mv.SIGNAL_WAKEUP, creation) == nil)
  assert(b.status == "running")
  self.signal_tree(self, mv.SIGNAL_WAKEUP, {relation = "ownership"})
  assert(not pcall(self.signal_tree, self, mv.SIGNAL_WAKEUP, {relation = "bad"}))

  assert(a:collision(b) == proc.collision(a, b))
  assert(a.collision(a, Actor) == proc.collision(a, Actor))
  assert(a:collision(b) == b)
  same_list(a:collisions(Actor), proc.collisions(a, Actor))
  same_list(a.collisions(a, b), proc.collisions(a, b))
  assert(not pcall(a.collision, a, nil) and not pcall(a.collisions, nil, b))
  local message = {value = 42}
  assert(select("#", a:send(message)) == 0)
  assert(select("#", proc.send(b, message)) == 0)
  assert(a:recv() == message and mv.recv(b) == message)
  a.send(a, "next")
  assert(a.recv(a) == "next" and self:recv() == nil)
  assert(not pcall(a.send, a, nil) and not pcall(a.recv))
  local ok, error_message = coroutine.resume(coroutine.create(function()
    assert(not pcall(mv.recv))
    assert(a:recv() == nil)
  end))
  assert(ok, error_message)

  if mv.animation then
    self.graph = mv.animation(mv.load_asset("../examples/aseprite/sheet.json"), "bounce")
    mv.frame()
    mv.frame()
    assert(self:animation_elapsed() > 0)
    assert(self.animation_elapsed(self) == mv.graphics.animation_elapsed(self))
    assert(not pcall(self.animation_elapsed))
  else
    assert(self.animation_elapsed == nil)
  end

  local ran_after_kill = false
  local SelfKill = proc(function(handle)
    handle:kill()
    ran_after_kill = true
  end)
  local suicide = SelfKill {}
  mv.frame()
  mv.frame()
  assert(not ran_after_kill and not suicide:alive() and suicide.reason == "killed")
  assert(suicide.status == "dead")
  assert(not pcall(function() return suicide.x end))
  assert(suicide:kill() == false and mv.kill(suicide) == false)
  assert(suicide:signal(mv.SIGNAL_KILL) == false)
  assert(not pcall(suicide.recv, suicide) and not pcall(mv.recv, suicide))
  assert(not pcall(suicide.children, suicide) and not pcall(proc.children, suicide))

  assert(a.kill(a) and proc.kill(b))
  assert(not a:alive() and not b:alive())
  assert(not pcall(a.set_priority, a, 6))
  assert(select("#", loose:kill_tree(creation)) == 0)
  assert(not loose:alive())
  local last = Actor {}
  assert(select("#", last.kill_tree(last)) == 0)
  assert(not last:alive())
end)

mv.start(function() Controller {} end)
