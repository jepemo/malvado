-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
return function()
  local keys = {"a", "left"}
  local index = 1
  local held, pressed, released, complete
  local function record(value)
    local file = assert(io.open("keyboard.txt", "ab"))
    assert(file:write(value))
    assert(file:close())
  end
  return function()
    if complete then return end
    assert(not mv.key("b") and not mv.key_pressed("b") and not mv.key_released("b"))
    if index > #keys then
      for _, key in ipairs(keys) do
        if mv.key(key) or mv.key_pressed(key) or mv.key_released(key) then return end
      end
      record("3")
      complete = true
      return
    end
    local key = keys[index]
    local down = mv.key(key)
    local press = mv.key_pressed(key)
    local release = mv.key_released(key)
    -- Reads share one logical tick. No query consumes the snapshot.
    assert(mv.key(key) == down and mv.key_pressed(key) == press and mv.key_released(key) == release)
    held, pressed, released = held or down, pressed or press, released or release
    if held and pressed and released and not down then
      record(tostring(index))
      index = index + 1
      held, pressed, released = false, false, false
    end
  end
end
