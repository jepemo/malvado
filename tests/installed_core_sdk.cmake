# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/game")
if(ARCHIVE_TEST)
  file(MAKE_DIRECTORY "${WORK}/archive" "${WORK}/sdk")
  execute_process(COMMAND "${CMAKE_COMMAND}" "-DBUILD=${BUILD}" "-DOUTPUT=${WORK}/archive"
    "-DCONFIG=${CONFIG}" -P "${SOURCE}/cmake/package-sdk.cmake" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "SDK archive creation failed")
  endif()
  execute_process(COMMAND "${CMAKE_COMMAND}" "-DBUILD=${BUILD}" "-DOUTPUT=${WORK}/archive"
    "-DCONFIG=${CONFIG}" -P "${SOURCE}/cmake/package-sdk.cmake" RESULT_VARIABLE repeat_result
    OUTPUT_QUIET ERROR_QUIET)
  if(repeat_result EQUAL 0)
    message(FATAL_ERROR "SDK packaging overwrote an existing archive")
  endif()
  file(GLOB archives "${WORK}/archive/*.tar.gz")
  list(LENGTH archives count)
  if(NOT count EQUAL 1)
    message(FATAL_ERROR "Expected one SDK archive")
  endif()
  list(GET archives 0 archive)
  if(NOT EXISTS "${archive}.sha256")
    message(FATAL_ERROR "SDK checksum is missing")
  endif()
  file(SHA256 "${archive}" actual_hash)
  file(READ "${archive}.sha256" checksum)
  string(FIND "${checksum}" "${actual_hash}" hash_position)
  if(NOT hash_position EQUAL 0)
    message(FATAL_ERROR "SDK checksum does not match")
  endif()
  execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xzf "${archive}"
    WORKING_DIRECTORY "${WORK}/sdk" RESULT_VARIABLE result)
else()
  execute_process(COMMAND "${CMAKE_COMMAND}" --install "${BUILD}" --prefix "${WORK}/sdk" --config "${CONFIG}"
    RESULT_VARIABLE result)
endif()
if(NOT result EQUAL 0)
  message(FATAL_ERROR "SDK preparation failed")
endif()
file(RENAME "${WORK}/sdk" "${WORK}/moved sdk")
set(notices "${WORK}/moved sdk/${DATADIR}/licenses/malvado")
if(NOT EXISTS "${notices}/LICENSE")
  message(FATAL_ERROR "Malvado license is missing")
endif()
if(HAS_ASSETS AND NOT EXISTS "${notices}/cJSON-LICENSE")
  message(FATAL_ERROR "cJSON license is missing")
endif()
if(HAS_GRAPHICS AND NOT EXISTS "${notices}/raylib-LICENSE")
  message(FATAL_ERROR "raylib license is missing")
endif()
if(CLI_NAME)
  if(NOT EXISTS "${notices}/Lua-readme.html")
    message(FATAL_ERROR "Lua license is missing")
  endif()
  execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DCLI=${WORK}/moved sdk/${BINDIR}/${CLI_NAME}" "-DWORK=${WORK}/cli"
    -P "${SOURCE}/tests/cli_project.cmake" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Installed client test failed")
  endif()
  execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DLAUNCHER=${WORK}/moved sdk/${LIBEXECDIR}/malvado/${LAUNCHER_NAME}"
    "-DWORK=${WORK}/launcher" -P "${SOURCE}/tests/game_launcher.cmake" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Installed game launcher test failed")
  endif()
  if(CAN_EXPORT)
    set(installed_cli "${WORK}/moved sdk/${BINDIR}/${CLI_NAME}")
    set(installed_launcher "${WORK}/moved sdk/${LIBEXECDIR}/malvado/${LAUNCHER_NAME}")
    execute_process(COMMAND "${installed_cli}" doctor RESULT_VARIABLE result OUTPUT_VARIABLE output)
    if(NOT result EQUAL 0 OR NOT output MATCHES "Export: launcher and license directory found")
      message(FATAL_ERROR "Doctor did not detect the installed SDK: ${output}")
    endif()
    file(RENAME "${installed_launcher}" "${installed_launcher}.saved")
    execute_process(COMMAND "${installed_cli}" doctor RESULT_VARIABLE result OUTPUT_VARIABLE output)
    file(RENAME "${installed_launcher}.saved" "${installed_launcher}")
    if(NOT result EQUAL 0 OR NOT output MATCHES "Export: launcher or license directory not found")
      message(FATAL_ERROR "Doctor did not detect a missing launcher: ${output}")
    endif()
    execute_process(COMMAND "${CMAKE_COMMAND}"
      "-DCLI=${WORK}/moved sdk/${BINDIR}/${CLI_NAME}"
      "-DWORK=${WORK}/export" "-DVISUAL=${RUN_GRAPHICS}"
      -P "${SOURCE}/tests/export_game.cmake" RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
      message(FATAL_ERROR "Game export test failed")
    endif()
  endif()
endif()
configure_file("${SOURCE}/examples/c/processes/main.c" "${WORK}/game/main.c" COPYONLY)
file(WRITE "${WORK}/game/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.20)
project(sdk_game C)
find_package(Malvado CONFIG REQUIRED COMPONENTS core)
add_executable(game main.c)
target_link_libraries(game PRIVATE Malvado::core)
enable_testing()
add_test(NAME game COMMAND game)
]=])
if(HAS_ASSETS)
  configure_file("${SOURCE}/tests/atlas.c" "${WORK}/game/atlas.c" COPYONLY)
  configure_file("${SOURCE}/tests/fixtures/atlas.json" "${WORK}/game/atlas.json" COPYONLY)
  configure_file("${SOURCE}/tests/fixtures/animated-atlas.json" "${WORK}/game/animated-atlas.json" COPYONLY)
  file(APPEND "${WORK}/game/CMakeLists.txt" [=[
find_package(Malvado CONFIG REQUIRED COMPONENTS assets)
add_executable(atlas_test atlas.c)
target_link_libraries(atlas_test PRIVATE Malvado::assets)
add_test(NAME atlas COMMAND atlas_test "${CMAKE_CURRENT_SOURCE_DIR}/atlas.json" "${CMAKE_CURRENT_SOURCE_DIR}/animated-atlas.json")
]=])
endif()
if(HAS_GRAPHICS)
  file(WRITE "${WORK}/game/graphics.c" [=[
#include "malvado/platform.h"
int main(void) {
  if (!mv_window_open(160, 100, "Installed SDK")) return 1;
  mv_window_begin((MvColor){0, 0, 0, 255});
  mv_window_draw_text("Hello", 10, 10, 20, (MvColor){255, 255, 255, 255});
  mv_window_end();
  mv_window_close();
  return 0;
}
]=])
  file(APPEND "${WORK}/game/CMakeLists.txt" [=[
find_package(Malvado CONFIG REQUIRED COMPONENTS graphics)
add_executable(graphics_test graphics.c)
target_link_libraries(graphics_test PRIVATE Malvado::graphics)
]=])
  if(RUN_GRAPHICS)
    file(APPEND "${WORK}/game/CMakeLists.txt" "add_test(NAME graphics COMMAND graphics_test)\n")
  endif()
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -S "${WORK}/game" -B "${WORK}/build"
  "-DCMAKE_PREFIX_PATH=${WORK}/moved sdk" "-DCMAKE_C_COMPILER=${C_COMPILER}"
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
