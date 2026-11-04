# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

function(check_script name script expected diagnostic)
  set(path "${CMAKE_CURRENT_BINARY_DIR}/start_result_${name}.lua")
  file(WRITE "${path}" "${script}\n")
  execute_process(COMMAND "${PROGRAM}" --headless "${path}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
  file(REMOVE "${path}")
  if(NOT result EQUAL expected OR NOT error MATCHES "${diagnostic}")
    message(FATAL_ERROR "${name}: status ${result}, output: ${output}, error: ${error}")
  endif()
endfunction()

check_script(empty "return {}" 0 "")
check_script(defaults "local mv = require('malvado'); return {init = function() local w, h = mv.window_size(); assert(w == 800 and h == 600) end}" 0 "")
check_script(nil "return nil" 0 "")
check_script(other "return 123" 0 "")
check_script(explicit "require('malvado').start(function() end); return 123" 0 "")
check_script(mixed "require('malvado').start(function() end); return {}" 1 "game table returned after start")
check_script(initializer "return {init = function() error('init failed') end}" 1 "init failed")
check_script(nested "return {init = function() require('malvado').start(function() end) end}" 1 "only be called once")
check_script(load "error('load failed')" 1 "load failed")
check_script(process "local mv = require('malvado'); local p = mv.process(function() error('task failed') end); return {init = function() p() end}" 1 "process failed")

set(path "${CMAKE_CURRENT_BINARY_DIR}/invalid_bootstrap.lua")
file(WRITE "${path}" "local mv = require('malvado'); local p = mv.process(function() print('unexpected tick') end); p(); return {width = 0}\n")
execute_process(COMMAND "${PROGRAM}" --headless "${path}"
  RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
file(REMOVE "${path}")
if(NOT result EQUAL 1 OR output MATCHES "unexpected tick")
  message(FATAL_ERROR "Invalid game ran a bootstrap process: ${result} ${output} ${error}")
endif()

set(path "${CMAKE_CURRENT_BINARY_DIR}/valid_bootstrap.lua")
file(WRITE "${path}" "local mv = require('malvado'); local p = mv.process(function() print('bootstrap tick') end); p(); return {}\n")
execute_process(COMMAND "${PROGRAM}" --headless "${path}"
  RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
file(REMOVE "${path}")
if(NOT result EQUAL 0 OR NOT output MATCHES "bootstrap tick")
  message(FATAL_ERROR "Bootstrap process did not run: ${result} ${output} ${error}")
endif()
