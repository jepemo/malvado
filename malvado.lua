--[[
  ______        _                _
 |  ___ \      | |              | |
 | | _ | | ____| |_   _ ____  _ | | ___
 | || || |/ _  | | | | / _  |/ || |/ _ \
 | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 |_||_||_|\_||_|_| \_/ \_||_|\____|\___/

 malvado - A game programming library with  "DIV Game Studio"-style
            processes for Lua/Love2D implemented in C with differents
            bindings for other languages.

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
    ident = -1,
    graph = nil,
    func = func,
    x = 0,
    y = 0,
    z = 0,
  }

  mtproc = {}

  mtproc.__call = function(args)
    --process.ident = engine.newProcId()
    --engine.addProc(process)
    -- Exit status
    return 0
  end

  setmetatable(process, mtproc)

  return process
end

-- #############################################################################
-- ENGINE
-- #############################################################################
local function Engine()
  local engine = {
    processes = {},
    proc_counter = 1,
  }

  engine.update = function (dt)
  end

  engine.keypressed = function(key)
  end

  engine.addProc = function(proc)
    table.insert(engine.processes, proc)
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
engine = Engine()

function key(code)
  return false
end

function process(func)
  return Process(engine, func)
end

function frame()
  coroutine.yield()
end

function kill(processToKill)
end

--[[
return {
  malvado = engine,
  process = mProcess,
  key = mKey,
  frame = mFrame,
  kill = mKill
}
]]--
