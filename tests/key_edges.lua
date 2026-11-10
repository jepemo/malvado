-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
assert(mv.input.key_pressed == mv.key_pressed)
assert(mv.input.key_released == mv.key_released)
for _, query in ipairs({mv.key_pressed, mv.key_released}) do
  for _, name in ipairs({"space", "a", "0", mv.KEY_ESCAPE}) do
    assert(query(name) == false and query(name) == false)
  end
  for _, name in ipairs({"", "SPACE", "space\0other", "unsupported"}) do
    assert(not pcall(query, name))
  end
  assert(not pcall(query, nil))
end
mv.import_globals()
assert(key_pressed == mv.key_pressed and key_released == mv.key_released)
local Worker = mv.process(function()
  for _ = 1, 3 do
    assert(not key_pressed("space") and not key_released("space"))
    mv.frame()
  end
end)
mv.start(function() Worker() end)
