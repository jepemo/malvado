# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

function(check_cleanup name body)
  set(path "${CMAKE_CURRENT_BINARY_DIR}/failure_cleanup_${name}.lua")
  file(WRITE "${path}" "${body}\n")
  execute_process(COMMAND "${PROGRAM}" --headless "${path}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
  file(REMOVE "${path}")
  if(NOT result EQUAL 1 OR NOT output STREQUAL "cleaned\n")
    message(FATAL_ERROR "${name}: status ${result}, output: ${output}, error: ${error}")
  endif()
endfunction()

set(guard "guard = setmetatable({}, {__gc = function() print('cleaned') end})")
check_cleanup(load "${guard}; error('load failure')")
check_cleanup(invalid "${guard}; return {width = 0}")
check_cleanup(init "${guard}; return {init = function() error('init failure') end}")
set(process [=[
local mv = require("malvado")
local task = mv.process(function()
  local guard <close> = setmetatable({}, {__close = function() print("cleaned") end})
  mv.frame()
  error("process failure")
end)
]=])
check_cleanup(explicit "${process}\nmv.start(function() task() end)")
check_cleanup(declarative "${process}\nreturn {init = function() task() end}")
