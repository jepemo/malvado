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
write("manifest.json", mv.fpg_json(fpg, true))
for index = 1, mv.fpg_count(fpg) do
  local id = mv.fpg_id_at(fpg, index)
  write(id .. ".png", assert(mv.fpg_png(fpg, id, true)))
end
