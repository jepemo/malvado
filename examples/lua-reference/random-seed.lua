-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

mv.rand_seed(12345)
local first_roll = mv.rand(1, 6)
mv.rand_seed(12345)
assert(mv.rand(1, 6) == first_roll)
print("Repeated seed gave the same roll:", first_roll)
mv.rand_seed()
