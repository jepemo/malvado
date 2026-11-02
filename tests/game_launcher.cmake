# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/game assets" "${WORK}/other")
get_filename_component(name "${LAUNCHER}" NAME)
configure_file("${LAUNCHER}" "${WORK}/game assets/${name}" COPYONLY)
file(WRITE "${WORK}/game assets/data.txt" "asset loaded")
if(VISUAL)
  file(WRITE "${WORK}/game assets/main.lua" [=[
local file = assert(io.open("data.txt"))
assert(file:read("a") == "asset loaded")
file:close()
local mv = require("malvado")
local game = mv.process(function()
  for _ = 1, 3 do mv.draw_text("Game", 10, 10, 20); mv.frame() end
end)
mv.start(function() game() end)
print("game loaded")
]=])
  set(args "")
else()
  file(WRITE "${WORK}/game assets/main.lua" [=[
local file = assert(io.open("data.txt"))
assert(file:read("a") == "asset loaded")
file:close()
local mv = require("malvado")
mv.start(function() print("game loaded") end)
]=])
  set(args "--headless")
endif()
execute_process(COMMAND "${WORK}/game assets/${name}" ${args} WORKING_DIRECTORY "${WORK}/other"
  RESULT_VARIABLE result OUTPUT_VARIABLE output)
if(NOT result EQUAL 0 OR NOT output MATCHES "game loaded")
  message(FATAL_ERROR "Relocated game launcher failed: ${output}")
endif()
file(REMOVE_RECURSE "${WORK}")
