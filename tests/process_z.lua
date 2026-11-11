-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")
local actor = mv.process(function(self)
  local expected = self.expected or 0
  assert(self.z == expected)
  assert(math.type(self.z) == math.type(expected))
  for _, value in ipairs({math.maxinteger, math.mininteger, 0.5, -0.0, 0}) do
    self.z = value
    assert(self.z == value and math.type(self.z) == math.type(value))
    assert(not pcall(function() self.z = math.huge end))
    assert(not pcall(function() self.z = "3" end))
    assert(self.z == value)
  end
  mv.frame()
  assert(self.z == 0)
end)
mv.start(function()
  actor()
  actor {z = math.maxinteger, expected = math.maxinteger}
end)
