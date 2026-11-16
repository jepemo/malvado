-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local task = mv.process(function()
  local guard <close> = setmetatable({}, {__close = function()
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
  local first_x, maximum_x, released
  local remaining = 60
  while os.getenv("MALVADO_APK_LIFECYCLE") or remaining > 0 do
    remaining = remaining - 1
    if os.getenv("MALVADO_APK_TOUCH") then
      if mv.mouse_button("left") then
        local x, y = mv.mouse_position()
        assert(not mv.mouse_button("middle") and not mv.mouse_button("right"))
        if not first_x then
          assert(math.abs(x - 160) <= 16 and math.abs(y - 120) <= 16)
          first_x, maximum_x = x, x
        end
        maximum_x = math.max(maximum_x, x)
      elseif first_x and not released then
        assert(maximum_x > first_x + 4)
        local file = assert(io.open("touch.txt", "wb"))
        assert(file:write("1"))
        assert(file:close())
        released = true
      end
    end
    mv.set_color(255, 0, 0)
    mv.draw_box(80, 60, 240, 180)
    mv.frame()
  end
  if os.getenv("MALVADO_APK_FAIL") then error("APK_EXPECTED_PROCESS_FAILURE") end
end)
return function() task() end
