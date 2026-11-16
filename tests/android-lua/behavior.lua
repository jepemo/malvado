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
  local remaining = 60
  while os.getenv("MALVADO_APK_LIFECYCLE") or remaining > 0 do
    remaining = remaining - 1
    mv.set_color(255, 0, 0)
    mv.draw_box(80, 60, 240, 180)
    mv.frame()
  end
  if os.getenv("MALVADO_APK_FAIL") then error("APK_EXPECTED_PROCESS_FAILURE") end
end)
return function() task() end
