# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

file(REMOVE_RECURSE "${WORK}")
execute_process(COMMAND "${CLI}" new "${WORK}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Could not create project")
endif()
execute_process(COMMAND "${HOST}" "${WORK}/main.lua" WORKING_DIRECTORY "${WORK}"
  RESULT_VARIABLE result)
file(REMOVE_RECURSE "${WORK}")
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Generated game did not run")
endif()
