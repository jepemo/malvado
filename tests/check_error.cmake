# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0
execute_process(COMMAND "${PROGRAM}" --headless "${SCRIPT}"
    RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 10)
if(NOT status MATCHES "^[0-9]+$" OR status EQUAL 0)
    message(FATAL_ERROR "Expected a normal nonzero exit, got '${status}': ${err}")
endif()
if(NOT err MATCHES "expected failure" OR NOT err MATCHES "stack traceback")
    message(FATAL_ERROR "Missing original error or traceback: ${err}")
endif()
