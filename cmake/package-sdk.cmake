# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

if(NOT BUILD OR NOT OUTPUT)
  message(FATAL_ERROR "Provide BUILD and OUTPUT directories")
endif()
get_filename_component(build "${BUILD}" ABSOLUTE)
get_filename_component(output "${OUTPUT}" ABSOLUTE)
set(stage "${output}/malvado-sdk-stage")
set(archive "${output}/malvado-sdk-alpha.4-dev.tar.gz")
if(EXISTS "${stage}" OR EXISTS "${archive}" OR EXISTS "${archive}.sha256")
  message(FATAL_ERROR "SDK output already exists in ${output}")
endif()
file(MAKE_DIRECTORY "${output}" "${stage}")
execute_process(COMMAND "${CMAKE_COMMAND}" --install "${build}" --prefix "${stage}" --config "${CONFIG}"
  RESULT_VARIABLE result)
if(result EQUAL 0)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E tar czf "${archive}" --format=gnutar .
    WORKING_DIRECTORY "${stage}" RESULT_VARIABLE result)
endif()
if(NOT result EQUAL 0)
  file(REMOVE_RECURSE "${stage}")
  message(FATAL_ERROR "SDK package creation failed")
endif()
file(SHA256 "${archive}" hash)
file(WRITE "${archive}.sha256" "${hash}  malvado-sdk-alpha.4-dev.tar.gz\n")
file(REMOVE_RECURSE "${stage}")
message(STATUS "Created ${archive}")
