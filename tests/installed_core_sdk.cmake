# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/game")
execute_process(COMMAND "${CMAKE_COMMAND}" --install "${BUILD}" --prefix "${WORK}/sdk" --config "${CONFIG}"
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "SDK installation failed")
endif()
configure_file("${SOURCE}/examples/c/processes/main.c" "${WORK}/game/main.c" COPYONLY)
file(WRITE "${WORK}/game/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.20)
project(sdk_game C)
find_package(Malvado CONFIG REQUIRED)
add_executable(game main.c)
target_link_libraries(game PRIVATE Malvado::core)
enable_testing()
add_test(NAME game COMMAND game)
]=])
execute_process(COMMAND "${CMAKE_COMMAND}" -S "${WORK}/game" -B "${WORK}/build"
  "-DCMAKE_PREFIX_PATH=${WORK}/sdk" "-DCMAKE_C_COMPILER=${C_COMPILER}"
  "-DCMAKE_C_FLAGS=${C_FLAGS}" "-DCMAKE_EXE_LINKER_FLAGS=${EXE_LINKER_FLAGS}"
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "SDK consumer configuration failed")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${WORK}/build" --config "${CONFIG}" RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "SDK consumer build failed")
endif()
execute_process(COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${WORK}/build" -C "${CONFIG}" --output-on-failure
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "SDK consumer run failed")
endif()
file(REMOVE_RECURSE "${WORK}")
