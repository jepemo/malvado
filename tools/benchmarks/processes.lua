-- Malvado 2; project created by Jeremías Pérez and contributors.
-- SPDX-License-Identifier: Apache-2.0

local mv = require("malvado")
local count = tonumber(os.getenv("MALVADO_BENCH_PROCESSES") or "100")
assert(count and count % 1 == 0 and count >= 1 and count <= 1000, "process count must be an integer in 1..1000")
local iterations = 60
local finished = 0
local spawn_seconds, start

local worker = mv.process(function()
  for _ = 1, iterations do mv.frame() end
  finished = finished + 1
end)

local monitor = mv.process(function()
  while finished < count do mv.frame() end
  local elapsed = mv.time_seconds() - start
  print(string.format("processes=%d yields_per_process=%d spawn_ms=%.3f run_ms=%.3f",
    count, iterations, spawn_seconds * 1000, elapsed * 1000))
end)

mv.start(function()
  local before = mv.time_seconds()
  for _ = 1, count do worker() end
  spawn_seconds = mv.time_seconds() - before
  start = mv.time_seconds()
  monitor()
end)
