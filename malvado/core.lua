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

--- Core module implements all the process workflow
-- @module malvado.core

--- [Internal] Process Object definition
-- @param engine ...
-- @param func ...
local function Process(engine, func)
  local process = {
    id = -1,
    -- Graphic
    graph = nil,
    -- Graphic package
    fpg = nil,
    -- Graphic index
    fpgIndex = -1,
    -- Process function
    func = nil,
    args = nil,
    x = 0,
    y = 0,
    z = 0,
    angle = 0,
    size = 1,
    state = 0,
    data_msg = {}
  }

  process.recv = function(self)
    if #self.data_msg > 0 then
      return table.remove(self.data_msg, 1)
    else
      return nil
    end
  end

  mtproc = {
  }

  mtproc.__call = function(t, args)
    -- print_v(t)

    args = args or {}

    new_proc = deepcopy(process)

    new_proc.id = engine.newProcId()
    new_proc.func = coroutine.create(func)
    new_proc.args = args

    --process = setmetatable(process, args)
    new_proc = setmetatable(new_proc, mtproc)

    engine.addProc(new_proc)
    debug("Created process:" .. new_proc.id)
    return new_proc.id
  end

  process = setmetatable(process, mtproc)

  return process
end

--- [Internal] Process engine
local function Engine()
  local engine = {
    processes = {},
    proc_counter = 1,
    started = false,
    keys = {},
    background_color = { r = 0, g = 0, b = 0 },
    messages = {}
  }

  local function render_process(process)
    local graphic = nil

    if process.fpg ~= nil
      and process.fpg.data ~= nil
      and #process.fpg.data > 0 then

      if self.fpgIndex ~= nil
        and type(self.fpgIndex) == 'number'
        and self.fpgIndex > 0
        and self.fpgIndex <= #process.fpg.data then

        graphic = process.fpg.data[self.fpgIndex]
      end
    elseif process.graph ~= nil then
      if process.graph.type == 'image' then
        graphic = process.graph.data
      end
    end

    if graphic ~= nil then
      local gwidth, gheight = graphic:getDimensions()
      love.graphics.draw(
        graphic,
        process.x,
        process.y,
        angleToRadians(process.angle),
        process.size,
        process.size,
        gwidth/2,
        gheight/2
      )
    end
  end

  engine.draw = function ()
    love.graphics.setBackgroundColor(
      engine.background_color.r,
      engine.background_color.g,
      engine.background_color.b)

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

    -- Esto hay que revisarlo
    table.sort(engine.processes, function(a, b)
      return a.z < b.z
    end)
  end

  engine.addProc = function(proc)
    table.insert(engine.processes, proc)
  end

  engine.newProcId = function()
    local newId = engine.proc_counter
    engine.proc_counter = engine.proc_counter + 1
    return newId
  end

  engine.kill = function(processToKill)
    local pos = 0
    for i, v in ipairs(engine.processes) do
      if v[id] == processToKill then
        pos = i
        break
      end
    end

    if pos > 0 then
      table.remove(engine.processes, pos)
    end
  end

  engine.send = function(proc_id, data)
    for i, v in ipairs(engine.processes) do
      if v.id == proc_id then
        table.insert(v.data_msg, data)
      end
    end
  end

  return engine
end

--- Engine instantiation.
malvado = Engine()

--- Define a process.
-- @param func the function definition of the process
-- @return The process definition (not the instance)
function process(func)
  return Process(malvado, func)
end

--- Stops the process executions and allow that itself can be rendered.
function frame()
  coroutine.yield()
end

--- Kills one process.
-- @param processToKill Unique identifier of the process, returned when is instantiated.
function kill(processToKill)
  malvado.kill(processToKill)
end

--- Sends a message to a process.
-- @param proc_id Unique identifier of the process, returned when is instantiated.
-- @param data Table with the data to send
function send(proc_id, data)
  malvado.send(proc_id, data)
end
