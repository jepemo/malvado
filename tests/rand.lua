-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
mv.import_globals()
assert(rand == mv.rand)
assert(rand_seed == mv.rand_seed)
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
math.randomseed(12345)
local expected = {}
for i = 1, 40 do
  local range = ranges[(i - 1) % #ranges + 1]
  expected[i] = math.random(range[1], range[2])
end
math.randomseed(12345)
for i = 1, #expected do
  local range = ranges[(i - 1) % #ranges + 1]
  local value = mv.rand(range[1], range[2])
  assert(value == expected[i] and value >= range[1] and value <= range[2])
end

-- Both functions advance the same generator even when interleaved.
math.randomseed(12345)
for i = 1, #expected do
  local range = ranges[(i - 1) % #ranges + 1]
  local draw = i % 2 == 0 and math.random or mv.rand
  assert(draw(range[1], range[2]) == expected[i])
end

-- Check delegation without assuming that two automatic seeds will differ.
local seed_calls = 0
debug.sethook(function()
  if debug.getinfo(2, "f").func == math.randomseed then
    seed_calls = seed_calls + 1
  end
end, "c")
assert(select("#", mv.rand_seed()) == 0)
debug.sethook()
assert(seed_calls == 1)
local roll = rand(1, 6)
assert(math.type(roll) == "integer" and roll >= 1 and roll <= 6)

local function sequence(mixed)
  local draws = {}
  for i = 1, 40 do
    local draw = mixed and i % 2 == 0 and math.random or rand
    draws[i] = draw(1, 100)
  end
  return table.concat(draws, ",")
end

assert(select("#", rand_seed(42)) == 0)
local repeated = sequence(false)
mv.rand_seed(42)
assert(sequence(false) == repeated)

for _, seed in ipairs({42, 0, -42, math.mininteger, math.maxinteger}) do
  math.randomseed(seed)
  local mixed = sequence(true)
  mv.rand_seed(seed)
  assert(sequence(true) == mixed)
end

local invalid = {
  table.pack(nil), table.pack(42, nil), table.pack(42, 0),
  table.pack(42.0), table.pack(1.5), table.pack(math.huge),
  table.pack("42"), table.pack(false), table.pack({}),
}
math.randomseed(42)
local mixed = sequence(true)
for _, args in ipairs(invalid) do
  math.randomseed(42)
  local ok, message = pcall(mv.rand_seed, table.unpack(args, 1, args.n))
  assert(not ok and message:find("integer", 1, true))
  assert(sequence(true) == mixed)
end
