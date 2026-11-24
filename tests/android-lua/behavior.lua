-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local task = mv.process(function()
  local audio
  local guard <close> = setmetatable({}, {__close = function()
    if audio then audio.finish() end
    local file = assert(io.open("closed.txt", "ab"))
    assert(file:write("1"))
    assert(file:close())
  end})
  local file = assert(io.open("message.bin", "rb"))
  assert(file:read("a") == string.char(0, 13, 10, 255))
  assert(file:close())
  local temporary = os.tmpname()
  assert(os.remove(temporary))
  local atlas = mv.load_asset("atlas.json")
  assert(mv.sprite_id(atlas, "red") == 7)
  if os.getenv("MALVADO_APK_AUDIO") then audio = require("audio")() end
  local keyboard
  if os.getenv("MALVADO_APK_KEYBOARD") then keyboard = require("keyboard")() end
  local first_x, maximum_x, released, moved
  local text_mode = os.getenv("MALVADO_APK_TEXT")
  local text = ""
  local remaining = 60
  while os.getenv("MALVADO_APK_LIFECYCLE") or remaining > 0 do
    remaining = remaining - 1
    if os.getenv("MALVADO_APK_TOUCH") then
      if released then assert(not mv.mouse_button(mv.MOUSE_LEFT)) end
      if mv.mouse_button(mv.MOUSE_LEFT) then
        local x, y = mv.mouse_position()
        assert(not mv.mouse_button(mv.MOUSE_MIDDLE) and not mv.mouse_button(mv.MOUSE_RIGHT))
        if not first_x then
          assert(mv.mouse_pressed(mv.MOUSE_LEFT) and mv.input.mouse_pressed(mv.MOUSE_LEFT))
          assert(math.abs(x - 160) <= 16 and math.abs(y - 120) <= 16)
          first_x, maximum_x = x, x
          local file = assert(io.open("touch-held.txt", "wb"))
          assert(file:write("1"))
          assert(file:close())
        end
        maximum_x = math.max(maximum_x, x)
        if maximum_x > first_x + 4 and not moved then
          local file = assert(io.open("touch-moved.txt", "wb"))
          assert(file:write("1"))
          assert(file:close())
          moved = true
        end
      elseif first_x and not released then
        assert(mv.mouse_released(mv.MOUSE_LEFT) and mv.input.mouse_released(mv.MOUSE_LEFT))
        assert(maximum_x > first_x + 4)
        local file = assert(io.open("touch.txt", "wb"))
        assert(file:write("1"))
        assert(file:close())
        released = true
      end
    end
    if text_mode then
      while true do
        local character = mv.text_input_next()
        if not character then break end
        text = text .. character
        assert(text == ("aZ0"):sub(1, #text))
        if text == "aZ0" then
          local marker = assert(io.open("text-input.txt", "wb"))
          assert(marker:write(text))
          assert(marker:close())
        end
      end
    end
    if audio then audio.frame() end
    if keyboard then keyboard() end
    mv.set_color {255, 0, 0, 255}
    mv.draw_rect {x = 80, y = 60, width = 160, height = 120}
    mv.frame()
  end
  if os.getenv("MALVADO_APK_FAIL") then error("APK_EXPECTED_PROCESS_FAILURE") end
end)
return function() task() end
