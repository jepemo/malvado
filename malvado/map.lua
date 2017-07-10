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
  -- Default values
  r = r or 255
  g = g or 255
  b = b or 255
  a = a or 255

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



FadeProcess = process(function(self)
  local in_progress = false
  local width = get_screen_width()
  local height = get_screen_height()

  local r = 0
  local r_end = 0

  local g = 0
  local g_end = 0

  local b = 0
  local b_end = 0

  local a = 0
  local a_end = 255

  local speed = 30
  self.z = 1000000

  while #malvado.processes > 1 do
    if in_progress == true then
      if r ~= r_end then
        if r_end < r then r = r - 1 else r = r + 1 end
      end
      if g ~= g_end then
        if g_end < r then g = g - 1 else g = g + 1 end
      end
      if b ~= b_end then
        if b_end < r then b = b - 1 else b = b + 1 end
      end

      if a ~= a_end then
        if a_end < a then a = a - 1 else a = a + 1 end
      end

      -- Reset the queue
      self.data_msg = {}

      if r == r_end and g == g_end and b == b_end and a == a_end then
        in_progress = false
      end
    else
      data = self:recv()

      if data ~= nil then
        --print_v(data)
        r_end = data.r
        g_end = data.g
        b_end = data.b
        a_end = data.a

        in_progress = true
      end
    end

    love.graphics.setColor(r, g, b, a)
    love.graphics.rectangle("fill", 0, 0, width, height)
    frame()
  end
end)

fade_process = FadeProcess {z = 1000000}

function fade(rc, gc, bc, ac, speedc)
  send(fade_process, { r = rc, g = gc, b = bc, a = ac, speed = speedc})
end

function fade_on()
  fade(0, 0, 0, 0, 16)
end

function fade_off()
  fade(0,0,0, 255, 16)
end
