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

function inc_value(current_value, maxmin_value, direction, increment)
  if direction == 1 and current_value >= maxmin_value then
    return 0
  end
  if direction == -1 and current_value <= maxmin_value then
    return 0
  end

  if direction == 1 and current_value < maxmin_value then
    return increment
  end

  if direction == -1 and current_value > maxmin_value then
    return -increment
  end
end

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

  local dir_r = 0
  local dir_g = 0
  local dir_b = 0
  local dir_a = 0
  local fades = {}

  print (self.id)

  while #malvado.processes > 1 do
    if in_progress == true then

      local inc_r = inc_value(r, r_end, dir_r, speed)
      local inc_g = inc_value(g, g_end, dir_g, speed)
      local inc_b = inc_value(b, b_end, dir_b, speed)
      local inc_a = inc_value(a, a_end, dir_a, speed)

      r = r + inc_r
      g = g + inc_g
      b = b + inc_b
      a = a + inc_a

      -- Reset the queue
      --self.data_msg = {}

      if inc_r == 0 and
         inc_g == 0 and
         inc_b == 0 and
         inc_a == 0 then
        in_progress = false
      end
    else
      --[[
      if #fades == 0 then
        data = self:recv()
        if data ~= nil then

        end
      end
      ]]--

      data = self:recv()

      if data ~= nil then
        print 'a'
        print_v(data)
        print 'b'
        r_end = data.r
        g_end = data.g
        b_end = data.b
        a_end = data.a
        speed = data.speed

        if r_end > r then dir_r = 1 else dir_r = -1 end
        if g_end > r then dir_g = 1 else dir_g = -1 end
        if b_end > r then dir_b = 1 else dir_b = -1 end
        if a_end > r then dir_a = 1 else dir_a = -1 end

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
  fade(0, 0, 0, 0, 5)
end

function fade_off()
  fade(0,0,0, 255, 5)
end
