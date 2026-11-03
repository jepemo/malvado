# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0
execute_process(COMMAND "${PROGRAM}" RESULT_VARIABLE first_status OUTPUT_VARIABLE first)
string(STRIP "${first}" first)
string(LENGTH "${first}" first_length)
if(NOT first_status EQUAL 0 OR NOT first_length EQUAL 48)
    message(FATAL_ERROR "Invalid first runtime identity: ${first}")
endif()
string(SUBSTRING "${first}" 0 16 high)
string(SUBSTRING "${first}" 16 16 low)
string(SUBSTRING "${first}" 32 16 sequence)
execute_process(COMMAND "${PROGRAM}" "${high}" "${low}" "${sequence}"
    RESULT_VARIABLE second_status OUTPUT_VARIABLE second)
string(STRIP "${second}" second)
string(LENGTH "${second}" second_length)
if(NOT first_status EQUAL 0 OR NOT second_status EQUAL 0 OR
   NOT first_length EQUAL 48 OR NOT second_length EQUAL 48 OR first STREQUAL second)
    message(FATAL_ERROR "Runtime identities are invalid or equal: ${first} / ${second}")
endif()
