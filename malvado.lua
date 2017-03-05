--
--  ______        _                _
-- |  ___ \      | |              | |
-- | | _ | | ____| |_   _ ____  _ | | ___
-- | || || |/ _  | | | | / _  |/ || |/ _ \
-- | || || ( ( | | |\ V ( ( | ( (_| | |_| |
-- |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
--
--

VERSION = 0.1

proc_counter = 0

local MalvadoProcess = {
  ident = 0,
  graph = nil,
  func = function() end,
  x = 0,
  y = 0,
  z = 0,
}

MalvadoProcess.__index = MalvadoProcess
MalvadoProcess.__call = function ()
  malvado.proc_count = malvado.proc_count + 1
  self.ident = malvado.proc_count
  table.insert(malvado.processes, proc)
end

function MalvadoProcess:new(ident, func)
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
