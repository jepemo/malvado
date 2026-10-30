# Malvado 2; created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

execute_process(COMMAND "${C_PROGRAM}" RESULT_VARIABLE c_result OUTPUT_VARIABLE c_output ERROR_VARIABLE c_error TIMEOUT 10)
execute_process(COMMAND "${LUA_PROGRAM}" --headless "${LUA_SCRIPT}" RESULT_VARIABLE lua_result OUTPUT_VARIABLE lua_output ERROR_VARIABLE lua_error TIMEOUT 10)
if(NOT c_result EQUAL 0 OR NOT lua_result EQUAL 0)
  message(FATAL_ERROR "Example failed: C=${c_result}: ${c_error}; Lua=${lua_result}: ${lua_error}")
endif()
if(NOT c_output STREQUAL lua_output)
  message(FATAL_ERROR "C and Lua traces differ:\nC:\n${c_output}\nLua:\n${lua_output}")
endif()
set(expected "controller: freeze\ncontroller: wake\nworker: ping\ncontroller: kill\nworker: closed\n")
if(NOT c_output STREQUAL expected)
  message(FATAL_ERROR "Unexpected trace:\n${c_output}")
endif()
