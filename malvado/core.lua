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
    delta = 1,
    data_msg = {},
    last_z = 0,
    fps = 30,
    time_per_frame = 0.03,
    current_frame_duration = 0,
    internal_process = false
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

    if (args.internal_process) then
      engine.n_internal_procs = engine.n_internal_procs + 1
    end

    debug("Created process:" .. new_proc.id)
    return new_proc.id
  end

  process = setmetatable(process, mtproc)

  return process
end

local function Engine()
  local engine = {
    processes = {},
    n_internal_procs = 0,
    proc_counter = 1,
    started = false,
    background_color = { r = 0, g = 0, b = 0 },
    messages = {},
    last_ms = 0,
  }

  local function render_process(process)
    local graphic = nil
    local anim_table = nil

    if process.fpg ~= nil then
      local fpg = process.fpg

      if fpg.type == 'image' then
        graphic = fpg.data
        anim_table = fpg.anim_table[(process.fpgIndex % #fpg.anim_table)+1]
      elseif fpg.type == 'directory' then
        graphic = fpg.data[(process.fpgIndex % #fpg.data)+1]
      end
      --[[
      and process.fpg.data ~= nil
      and #process.fpg.data > 0 then

      if self.fpgIndex ~= nil
        and type(self.fpgIndex) == 'number'
        and self.fpgIndex > 0
        and self.fpgIndex <= #process.fpg.data then

        graphic = process.fpg.data[self.fpgIndex]
        anim_table = process.fpg.
      end
      ]]--
    elseif process.graph ~= nil then
      if process.graph.type == 'image' then
        graphic = process.graph.data
      end
    end

    if graphic ~= nil then
      local gwidth, gheight = graphic:getDimensions()

      set_color(255, 255, 255, 255)
      if anim_table ~= nil then
        love.graphics.draw (
          graphic,
          anim_table,
          process.x,
          process.y,
          angleToRadians(process.angle),
          process.size,
          process.size
          --gwidth/2,
          --gheight/2
        )
      else
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
  end

  engine.draw = function ()
    local dt = 1
    local z_changed = false

    love.graphics.setBackgroundColor(
      engine.background_color.r,
      engine.background_color.g,
      engine.background_color.b)

    local dt = love.timer.getDelta( )
    to_delete = {}
    for i,v in ipairs(engine.processes) do
      if v.state == 0 then
        for k2,v2 in pairs(v.args) do
          v[k2] = v2
        end
        v.state = 1
      end

      v.time_per_frame = 1.0 / v.fps
      v.delta = dt
      v.current_frame_duration = v.current_frame_duration + dt

      local execute_process = (v.current_frame_duration >= v.time_per_frame)
      if execute_process then
        v.current_frame_duration = 0
        local ok, error = coroutine.resume(v.func, v)
        if not ok then
          debug(error)
        end
      end

      if (v.graph ~= nil or v.fpg ~= nil) then
        render_process(v)
      end

      if coroutine.status(v.func) == "dead" then
        debug("Finalized process:" .. v.id)
        table.insert(to_delete, i)
      end

      if not z_changed and v.z ~= v.last_z then
        z_changed = true
      end

      v.last_z = v.z
    end

    if #to_delete > 0 then
      for i,v in ipairs(to_delete) do
        local proc = engine.processes[v]
        if proc[internal_process] ~= nil and proc.internal_process then
          engine.n_internal_procs = engine.n_internal_procs - 1
        end

        table.remove(engine.processes, v)
      end
    end

    if #engine.processes == 0 or #engine.processes <= engine.n_internal_procs then
      love.event.quit(0)
    end

    if z_changed then
      table.sort(engine.processes, function(a, b)
        return a.z < b.z
      end)
    end
  end

  engine.addProc = function(proc)
    engine.processes[proc.id] = proc
  end

  engine.newProcId = function()
    local newId = engine.proc_counter
    engine.proc_counter = engine.proc_counter + 1
    return newId
  end

  engine.kill = function(processToKill)
    if engine.processes[processToKill] ~= nil then
      engine.processes[processToKill] = nil
    end
  end

  engine.send = function(proc_id, data)
    if engine.processes[proc_id] ~= nil then
      proc = engine.processes[proc_id]
      table.insert(proc.data_msg, data)
    end
  end

  --[[
  engine.add_text = function(text_obj)
    local textId = text_obj.id
    engine.texts[textId] = text_obj
  end
  engine.delete_text = function(text_id)
    engine.texts[text_id] = nil
  end
  ]]--

  engine.start = function(init)
    -- Init te timer
    engine.last_ms = os.time()

    love.draw = engine.draw

    if init ~= nil then
      init()
    end
  end

  engine.mod_process = function(proc_id, values)
    proc = engine.processes[proc_id]

    for i,v in ipairs(values) do
      proc[i] = v
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

--- Exits from the application
-- @param statusCode Exit status code. Default value 0
function exit(statusCode)
  statusCode = statusCode or 0
  love.event.quit(statusCode)
end

--- Change process internal values
-- @param proc_id Process id
-- @param values Object with values to change
function set_proc(proc_id, values)
  malvado.mod_process(proc_id, values)
end
