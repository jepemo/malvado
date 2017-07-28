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
    -- PUBLIC PROPERTIES ------------------------------------------------------

    -- Process identifier
    id = -1,
    -- Graphic
    graph = nil,
    -- Graphic package
    fpg = nil,
    -- Graphic index
    fpgIndex = -1,
    -- Process parent
    parent = nil,
    -- X pos
    x = 0,
    -- Y pos
    y = 0,
    -- Z pos
    z = 0,
    -- Angle
    angle = 0,
    -- Size
    size = 1,
    -- Width
    width = 0,
    -- Height
    height = 0,
    -- Delta (fps)
    delta = 1,
    -- Frames per second
    fps = 0,

    -- PRIVATE PROPERTIES -----------------------------------------------------

    -- Process function
    _func = nil,
    -- Process arguments
    _args = nil,
    -- Process state
    _state = 0,
    -- Message box
    _data_msg = {},
    -- Z position last frame
    _last_z = 0,
    -- delta adjuster
    _time_per_frame = 0.03,
    -- Current frame duration
    _current_frame_duration = 0,
    -- If it a special system process
    _internal = false,
    -- If a process have childs, to destroy when the parent is destroyed
    _children = nil,
  }

  --- Non blocking message receiver
  process.recv = function(self)
    if #self._data_msg > 0 then
      return table.remove(self._data_msg, 1)
    else
      return nil
    end
  end

  -- Process metaclass
  mtproc = {
  }

  -- When a process is created
  mtproc.__call = function(t, args)
    args = args or {}

    new_proc = deepcopy(process)

    new_proc.id = engine.newProcId()
    new_proc._func = coroutine.create(func)
    new_proc._args = args

    new_proc = setmetatable(new_proc, mtproc)

    engine.addProc(new_proc)

    if (args._internal) then
      engine.n_internal_procs = engine.n_internal_procs + 1
    end

    debug("Created process:" .. new_proc.id)
    return new_proc.id
  end

  process = setmetatable(process, mtproc)

  return process
end

-- Represents the mouse
mouse = {
  -- X pos
  x = 0,
  -- Y position
  y = 0,
  -- Graphic collection
  fpg = 0,
  -- Graphic of grapic collection
  fpgIndex = 0,
  -- Cursor graphic
  graph = nil
}

local function render_mouse()
  -- Nothing at te moment
end

local function Engine()
  local engine = {
    processes = {},
    n_procs = 0,
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
      elseif fpg.type == 'zip' then
        error("FPG zip mode is not available")
      end
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
    -- Reset vars
    local dt = love.timer.getDelta( )
    local to_delete = {}
    local z_changed = false

    scan_code = 0

    mouse.x = love.mouse.getX()
    mouse.y = love.mouse.getY()

    render_mouse()

    -- Clear screen
    love.graphics.setBackgroundColor(
      engine.background_color.r,
      engine.background_color.g,
      engine.background_color.b)

    -- For every process
    for id, proc in pairs(engine.processes) do
      -- Pass process arguments
      if proc._state == 0 then
        engine.mod_process(id, proc._args)
        proc._state = 1
      end

      -- Update frame vars
      local execute_process = true
      if proc.fps ~= 0 then
        proc._time_per_frame = 1.0 / proc.fps
        proc.delta = dt
        proc._current_frame_duration = proc._current_frame_duration + dt

        execute_process = (proc._current_frame_duration >= proc._time_per_frame)
      end

      -- Execute process
      if execute_process or proc.internal then
        proc._current_frame_duration = 0
        local ok, error = coroutine.resume(proc._func, proc)
        if not ok then
          debug(error)
        end
      end

      -- Render the process if as a graphic
      if (proc.graph ~= nil or proc.fpg ~= nil) then
        render_process(proc)
      end

      -- If the process have ended, mark to delete
      if coroutine.status(proc._func) == "dead" then
        debug("Finalized process:" .. id)
        table.insert(to_delete, id)
      end

      -- Check if some Z-depth have been updated
      if not z_changed and proc.z ~= proc._last_z then
        z_changed = true
      end

      -- Update te z-depth
      proc._last_z = proc.z
    end

    -- Delete the processes that are finished
    if #to_delete > 0 then
      for _, id in ipairs(to_delete) do
        engine.kill(id)
      end
    end

    -- Exit the application if there aren't active processes
    if engine.n_procs == 0 or engine.n_procs <= engine.n_internal_procs then
      love.event.quit(0)
    end

    -- Re-order the processes if some z-depth have been changed
    if z_changed then
      engine.update_zdepths()
    end
  end

  engine.update_zdepths = function()
    debug('Recalcula z...')
    table.sort(engine.processes, function(a, b)
      return a.z < b.z
    end)
  end

  engine.addProc = function(proc)
    engine.processes[ident(proc.id)] = proc
    engine.n_procs = tlen(engine.processes)

    engine.update_zdepths()

    -- Update parent/children
    if proc._args.parent ~= nil then
      print 'entra!!!'
      if proc._args.parent._children == nil then
        proc._args.parent._children = {}
      end

      table.insert(proc._args.parent._children, proc.id)
    end
  end

  engine.newProcId = function()
    local newId = engine.proc_counter
    engine.proc_counter = engine.proc_counter + 1
    return newId
  end

  engine.kill = function(processToKill)
    if engine.processes[ident(processToKill)] ~= nil then
      local proc_del = engine.processes[ident(processToKill)]
      if proc_del["_internal"] ~= nil and proc_del["_internal"] == true then
        engine.n_internal_procs = engine.n_internal_procs - 1
      end

      engine.processes[ident(processToKill)] = nil

      -- Delete children processses
      if (proc_del._children ~= nil and #proc_del._children > 0) then
        for ind, cid in ipairs(proc_del._children) do
          engine.kill(cid)
        end
      end
    end

    engine.n_procs = tlen(engine.processes)
  end

  engine.send = function(proc_id, data)
    if engine.processes[ident(proc_id)] ~= nil then
      proc = engine.processes[ident(proc_id)]
      table.insert(proc._data_msg, data)
    end
  end

  --- Start the game program
  -- @param init Initial function
  -- @param debug_activated Debug mode, default false
  engine.start = function(init, debug_activated)
    debug_mode = debug_activated or false
    debug("Start")

    -- Init te timer
    engine.last_ms = os.time()

    love.draw = engine.draw

    if init ~= nil then
      init()
    end
  end

  engine.mod_process = function(proc_id, values)
    proc = engine.processes[ident(proc_id)]
    for k,v in pairs(values) do
      proc[k] = v
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
