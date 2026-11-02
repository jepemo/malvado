# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

file(REMOVE_RECURSE "${WORK}")
execute_process(COMMAND "${CLI}" new "${WORK}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Could not create project")
endif()
execute_process(COMMAND "${HOST}" "${WORK}/main.lua" WORKING_DIRECTORY "${WORK}"
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Generated game did not run")
endif()
file(WRITE "${WORK}/asset.txt" "asset")
file(WRITE "${WORK}/main.lua" [=[
local file = assert(io.open("asset.txt"))
assert(file:read("a") == "asset")
file:close()
local mv = require("malvado")
local game = mv.process(function()
  for _ = 1, 3 do mv.draw_text("Hello", 10, 10, 20); mv.frame() end
end)
mv.start(function() game() end)
]=])
execute_process(COMMAND "${CLI}" run "${WORK}" WORKING_DIRECTORY "${WORK}/.."
  RESULT_VARIABLE result)
file(REMOVE_RECURSE "${WORK}")
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Game did not run from its project directory")
endif()
