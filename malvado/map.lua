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

function font(size, r, g, b, a)
  return {
    font = love.graphics.newFont(size),
    r = r,
    g = g,
    b = b,
    a = a
  }
end

--------------------------------------------------------------------------------
-- FADE
--------------------------------------------------------------------------------



fade_process = nil
FadeProcess = process(function(self)
  local in_progress = true
  local width = get_screen_width()
  local height = get_screen_height()
  local red = 0
  local green = 0
  local blue = 0
  local alpha = 0

  while true do
    if in_progress then
      love.graphics.setColor(red, green, blue, alpha )
    end

    love.graphics.rectangle("fill", 0, 0, width, height)

    frame()
  end
end)

function fade(r, g, b, speed)
  if fade_process == nil then
    fade_process = FadeProcess { z = 1000000, rcomp = r, gcomp = g, bcomp = b, speed = speed }
  else
    -- Change
  end
end

function fade_on()
  fade(100,100,100,16)
end

function fade_off()
  fade(0,0,0,16)
end
