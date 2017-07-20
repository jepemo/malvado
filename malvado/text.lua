--[[
  ______        _                _
 |  ___ \      | |              | |
 | | _ | | ____| |_   _ ____  _ | | ___
 | || || |/ _  | | | | / _  |/ || |/ _ \
 | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 |_||_||_|\_||_|_| \_/ \_||_|\____|\___/

 malvado - A game programming library with  "DIV Game Studio"-style
            processes for Lua/Love2D.

 Copyright (C) 2017-present Jeremies Pérez Morata

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
--]]

--- Text primitives
-- @module malvado.text

local textId = 0

function render_text(text)
  local font = text.font
  local _text = text.text
  local x    = text.x
  local y    = text.y

  love.graphics.setFont(font.font)
  love.graphics.setColor(font.r, font.g, font.b, 255)
  love.graphics.print(_text, x, y)
end

--- Create a text to render
-- @param font Font object
-- @param _x x
-- @param _y y
-- @param _text text
-- @see malvado.map.font
function write(font, _x, _y, _text)
  textId = textId + 1
  text_obj = {
    id = textId,
    font = font,
    x = _x,
    y = _y,
    text = _text
  }

  malvado.add_text(text_obj)

  return textId
end

--- Delete a text by its identifier
-- @param id text id
function delete_text(id)
  if id ~= nil then
    malvado.delete_text(id)
  end
end
