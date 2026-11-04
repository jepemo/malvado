# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

set(cases "{width = 0}" "{height = 4294967297}" "{title = 12}" "{init = true}" "{unknown = 1}" "setmetatable({}, {})")
foreach(value IN LISTS cases)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/invalid_game.lua" "return ${value}\n")
  execute_process(COMMAND "${PROGRAM}" --headless "${CMAKE_CURRENT_BINARY_DIR}/invalid_game.lua"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
  if(NOT result EQUAL 1)
    message(FATAL_ERROR "Accepted invalid game ${value}: ${result} ${output} ${error}")
  endif()
endforeach()
file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/invalid_game.lua")
