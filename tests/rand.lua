-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
assert(rand == mv.rand)
assert(mv.rand(7, 7) == 7)
assert(mv.rand(math.mininteger, math.mininteger) == math.mininteger)
assert(mv.rand(math.maxinteger, math.maxinteger) == math.maxinteger)
assert(not pcall(mv.rand, 2, 1))
assert(not pcall(mv.rand, 1.5, 3))
assert(not pcall(mv.rand, 0, math.huge))

-- Rejected ranges do not advance the shared Lua generator.
math.randomseed(12345, 67890)
local first = math.random(1, 100)
math.randomseed(12345, 67890)
assert(not pcall(mv.rand, 2, 1))
assert(mv.rand(1, 100) == first)

local ranges = {{-5, 5}, {0, 1}, {math.mininteger, math.maxinteger}, {1, 100}}
math.randomseed(12345, 67890)
local expected = {}
for i = 1, 40 do
  local range = ranges[(i - 1) % #ranges + 1]
  expected[i] = math.random(range[1], range[2])
end
math.randomseed(12345, 67890)
for i = 1, #expected do
  local range = ranges[(i - 1) % #ranges + 1]
  local value = mv.rand(range[1], range[2])
  assert(value == expected[i] and value >= range[1] and value <= range[2])
end
