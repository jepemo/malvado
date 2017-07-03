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
-- UTILITIES
-- #############################################################################
local function angleToRadians (angle)
  return (angle * math.pi) / 180
end
local function debug(text)
  print(text)
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
    size = 1,
    state = 0,
  }

  mtproc = {
  }

  mtproc.__call = function(t, args)
    process.id = engine.newProcId()
    process.func = coroutine.create(func)
    process.args = args

    process = setmetatable(process, args)

    debug("Created process:" .. process.id)

    engine.addProc(process)
    return 0
  end

  process = setmetatable(process, mtproc)

  return process
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

  local function render_process(process)
    if type(process.graph) == 'userdata' then
      local gwidth, gheight = process.graph:getDimensions()

      love.graphics.draw(
        process.graph,
        process.x,
        process.y,
        --process.angle,
        angleToRadians(process.angle),
        process.size,
        process.size,
        gwidth/2,
        gheight/2
      )
    end
  end

  engine.draw = function ()
    to_delete = {}
    for i,v in ipairs(engine.processes) do
      if v.state == 0 then
        for k2,v2 in pairs(v.args) do
          v[k2] = v2
        end
        v.state = 1
      end

      local ok, error = coroutine.resume(v.func, v)
      if not ok then
        debug(error)
      end

      if (v.graph ~= nil) then
        render_process(v)
      end

      if coroutine.status(v.func) == "dead" then
        debug("Finalized process:" .. v.id)
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
  return love.keyboard.isDown(code)
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

function cos(angle)
  return math.cos(angleToRadians(angle))
end
function sin(angle)
  return math.sin(angleToRadians(angle))
end

function write(font, x, y, text)
  love.graphics.setFont(font.font)
  love.graphics.setColor(font.r, font.g, font.b, font.a)
  love.graphics.print(text, x, y)
end

function set_title(title)
  love.window.setTitle(title)
end
function get_screen_width()
  local screen_width, _ = love.graphics.getDimensions()
  return screen_width
end
function get_screen_height()
  local _, screen_height = love.graphics.getDimensions()
  return screen_height
end
