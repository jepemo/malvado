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

local proc_counter = 0

local MalvadoProcess = {
  ident = 0,
  graph = nil,
  func = function() end,
  x = 0,
  y = 0,
  z = 0,
}

MalvadoProcess.__call = function ()
  malvado.proc_count = malvado.proc_count + 1
  --self.ident = malvado.proc_count
  --table.insert(malvado.processes, proc)
end

function MalvadoProcess:new(ident, func)
  self.__index = self
  return setmetatable({
    ident = ident,
    func = func,
  }, MalvadoProcess)
end

local MalvadoEngine = {}
MalvadoEngine.__index = MalvadoEngine

function MalvadoEngine:new()
  return setmetatable({
    processes = {},
    proc_count = 0,
  }, MalvadoEngine)
end

function MalvadoEngine:add_proc(func)
  proc = MalvadoProcess:new(func)
  return proc
end

function MalvadoEngine:keypressed(key)
end

function MalvadoEngine:update(dt)
end

--function MalvadoEngine:key_pressed(key)
--end

malvado = MalvadoEngine:new()

-- #############################################################################
-- LIBRARY INTERFACE
-- #############################################################################
function key(code)
  --return malvado:key_pressed(code)
  return false
end

function process(func)
  return malvado:add_proc(func)
end

function kill(processToKill)
end
