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

VERSION = 0.1

-- #############################################################################
-- PROCESS
-- #############################################################################
local function Process(engine, func)
  local process = {
    id = -1,
    graph = nil,
    func = nil,
    args = nil,
    x = 0,
    y = 0,
    z = 0,
    angle = 0,
    size = 100,
  }

  mtproc = {
  }

  mtproc.__call = function(t, args)
    process.id = engine.newProcId()
    process.func = coroutine.create(func)
    process.args = args

    process = setmetatable(process, args)
    --print_v(process)

    engine.addProc(process)
    return 0
  end

  process = setmetatable(process, mtproc)

  return process
end

function print_v(o) print(dump(o)) end
function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end


-- #############################################################################
-- ENGINE
-- #############################################################################
local function Engine()
  local engine = {
    processes = {},
    proc_counter = 1,
    started = false,
    keys = {},
  }

  engine.draw = function ()
    if not engine.started then
      for i,v in ipairs(engine.processes) do
        -- Merge tables
        for k2,v2 in pairs(v.args) do
          v[k2] = v2
        end

        coroutine.resume(v.func, v)
      end
      engine.started = true
    end

    to_delete = {}
    for i,v in ipairs(engine.processes) do
      coroutine.resume(v.func, v)

      if coroutine.status(v.func) == "dead" then
        table.insert(to_delete, i)
      end
    end

    if #to_delete > 0 then
      for i,v in ipairs(to_delete) do
        table.remove(engine.processes, v)
      end
    end

    if #engine.processes == 0 then
      love.event.quit(0)
    end

    engine.keys = {}
  end

  engine.keypressed = function(key)
    engine.keys[key] = true
  end

  engine.addProc = function(proc)
    table.insert(engine.processes, proc)

    table.sort(engine.processes, function(a, b)
      return a.z < b.z
    end)
  end

  engine.newProcId = function()
    local newId = engine.proc_counter
    engine.proc_counter = engine.proc_counter + 1
    return newId
  end

  return engine
end

-- #############################################################################
-- LIBRARY INTERFACE
-- #############################################################################
malvado = Engine()

function key(code)
  return malvado.keys[code] ~= nil and malvado.keys[code]
end

function process(func)
  return Process(malvado, func)
end

function frame()
  coroutine.yield()
end

function kill(processToKill)
end

function font(size, r, g, b, a)
  return {
    font = love.graphics.newFont(size),
    r = r,
    g = g,
    b = b,
    a = a
  }
end

function write(font, x, y, text)
  love.graphics.setFont(font.font)
  love.graphics.setColor(font.r, font.g, font.b, font.a)
  love.graphics.print(text, x, y)
end

function set_title(title)
  love.window.setTitle(title)
end
