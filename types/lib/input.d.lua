-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0
--
-- Input and audio types for the Lua Language Server (LuaLS).

---@meta

---Key name accepted by the input functions.
---Letters are single characters, like `KEY_A == "a"`.
---@alias MvKey string
---|"a"|"b"|"c"|"d"|"e"|"f"|"g"|"h"|"i"|"j"|"k"|"l"|"m"
---|"n"|"o"|"p"|"q"|"r"|"s"|"t"|"u"|"v"|"w"|"x"|"y"|"z"
---|"0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
---|"escape"|"space"|"left"|"right"|"up"|"down"|"enter"|"tab"|"backspace"

---Mouse button name.
---@alias MvMouseButton "left"|"middle"|"right"
---@alias MvPadButton "a"|"b"|"x"|"y"|"up"|"down"|"left"|"right"|"lb"|"rb"|"back"|"start"
---@alias MvPadAxis "left_x"|"left_y"|"right_x"|"right_y"|"left_trigger"|"right_trigger"
