-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local input = assert(io.open("tests/fixtures/fpg/bennugd-8.fpg", "rb"))
local fpg = mv.fpg.parse(input:read("a"))
input:close()
local directory = assert(os.getenv("MALVADO_FPG_TEST_DIR"))
local function write(name, bytes)
  local file = assert(io.open(directory .. "/" .. name, "wb"))
  assert(file:write(bytes))
  assert(file:close())
end
write("manifest.json", fpg:json(true))
for index = 1, fpg:count() do
  local id = fpg:id_at(index)
  write(id .. ".png", assert(fpg:png(id, true)))
end
