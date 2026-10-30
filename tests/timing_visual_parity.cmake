# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "MALVADO_CAPTURE_PATH=${CAPTURE}" "${C_DEMO}"
  RESULT_VARIABLE c_result)
if(NOT c_result EQUAL 0)
  message(FATAL_ERROR "C timing game capture failed: ${c_result}")
endif()
execute_process(
  COMMAND "${LUA_TEST}" "${LUA_DEMO}" "${CAPTURE}"
  RESULT_VARIABLE lua_result)
file(REMOVE "${CAPTURE}")
if(NOT lua_result EQUAL 0)
  message(FATAL_ERROR "C and Lua timing game frames differ: ${lua_result}")
endif()
