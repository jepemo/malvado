# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

set(prefix "${CMAKE_CURRENT_BINARY_DIR}/entry_parity")
set(common [=[
local mv = require("malvado")
local task = mv.process(function()
  print("tick 1")
  mv.frame()
  print("tick 2")
end)
local function init()
  print("init")
  task()
end
]=])
file(WRITE "${prefix}_explicit.lua" "${common}\nmv.start(init)\n")
file(WRITE "${prefix}_declared.lua" "${common}\nreturn {init = init}\n")
foreach(style explicit declared)
  execute_process(COMMAND "${PROGRAM}" --headless "${prefix}_${style}.lua"
    RESULT_VARIABLE status_${style} OUTPUT_VARIABLE output_${style} ERROR_VARIABLE error_${style})
  file(REMOVE "${prefix}_${style}.lua")
endforeach()
if(NOT status_explicit EQUAL 0 OR NOT status_declared EQUAL 0 OR
   NOT output_explicit STREQUAL output_declared OR
   NOT output_explicit STREQUAL "init\ntick 1\ntick 2\n")
  message(FATAL_ERROR "Entry styles differ: ${status_explicit} ${output_explicit} ${error_explicit}; ${status_declared} ${output_declared} ${error_declared}")
endif()

foreach(failure init process)
  if(failure STREQUAL "init")
    set(common "local function init() error('init failure') end")
  else()
    set(common "local mv = require('malvado'); local task = mv.process(function() error('process failure') end); local function init() task() end")
  endif()
  file(WRITE "${prefix}_explicit.lua" "${common}\nrequire('malvado').start(init)\n")
  file(WRITE "${prefix}_declared.lua" "${common}\nreturn {init = init}\n")
  foreach(style explicit declared)
    execute_process(COMMAND "${PROGRAM}" --headless "${prefix}_${style}.lua"
      RESULT_VARIABLE status_${style} OUTPUT_VARIABLE output_${style} ERROR_VARIABLE error_${style})
    file(REMOVE "${prefix}_${style}.lua")
  endforeach()
  if(NOT status_explicit EQUAL 1 OR NOT status_declared EQUAL 1 OR
     NOT error_explicit MATCHES "${failure} fail" OR NOT error_declared MATCHES "${failure} fail")
    message(FATAL_ERROR "${failure} exit differs: ${status_explicit} ${error_explicit}; ${status_declared} ${error_declared}")
  endif()
endforeach()
