# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

execute_process(COMMAND "${PROGRAM}" "${SCRIPT}"
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 10)
string(REGEX MATCHALL "native shutdown handle valid" matches "${output}")
list(LENGTH matches count)
if(NOT status EQUAL 0 OR NOT count EQUAL 2 OR NOT error MATCHES "force native shutdown")
  message(FATAL_ERROR "Native shutdown check failed: ${status} ${output} ${error}")
endif()
