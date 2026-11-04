# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

execute_process(COMMAND "${PROGRAM}" --headless "${SCRIPT}"
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 10)
if(NOT status EQUAL 1 OR NOT error MATCHES "force shutdown" OR
   NOT output MATCHES "shutdown handles valid")
  message(FATAL_ERROR "Shutdown handle check failed: ${status} ${output} ${error}")
endif()
