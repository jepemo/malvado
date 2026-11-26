-- Malvado 2; SPDX-License-Identifier: Apache-2.0
local mv = require("malvado")

mv.import_globals()
mv.import_globals() -- Importing the same exports again is safe.

assert(frame == mv.frame)
assert(KEY_ESCAPE == mv.KEY_ESCAPE)
print("Malvado globals installed")
