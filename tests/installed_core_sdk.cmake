# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/game")
if(ARCHIVE_TEST)
  file(MAKE_DIRECTORY "${WORK}/archive" "${WORK}/sdk")
  execute_process(COMMAND "${CMAKE_COMMAND}" "-DBUILD=${BUILD}" "-DOUTPUT=${WORK}/archive"
    "-DCONFIG=${CONFIG}" -P "${SOURCE}/tools/cmake/package-sdk.cmake" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "SDK archive creation failed")
  endif()
  execute_process(COMMAND "${CMAKE_COMMAND}" "-DBUILD=${BUILD}" "-DOUTPUT=${WORK}/archive"
    "-DCONFIG=${CONFIG}" -P "${SOURCE}/tools/cmake/package-sdk.cmake" RESULT_VARIABLE repeat_result
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
  file(STRINGS "${BUILD}/malvado-sdk-target.txt" sdk_target)
  list(GET sdk_target 0 sdk_version)
  list(GET sdk_target 1 sdk_system)
  list(GET sdk_target 2 sdk_arch)
  get_filename_component(archive_name "${archive}" NAME)
  if(NOT archive_name STREQUAL "malvado-sdk-${sdk_version}-${sdk_system}-${sdk_arch}.tar.gz")
    message(FATAL_ERROR "SDK archive name does not identify its version and target")
  endif()
  if(NOT EXISTS "${archive}.sha256")
    message(FATAL_ERROR "SDK checksum is missing")
  endif()
  file(SHA256 "${archive}" actual_hash)
  file(READ "${archive}.sha256" checksum)
  get_filename_component(archive_name "${archive}" NAME)
  if(NOT checksum STREQUAL "${actual_hash}  ${archive_name}\n")
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
file(STRINGS "${BUILD}/malvado-sdk-target.txt" sdk_target)
list(GET sdk_target 0 sdk_version)
list(GET sdk_target 1 sdk_system)
list(GET sdk_target 2 sdk_arch)
set(sdk_info "${WORK}/moved sdk/${DATADIR}/malvado/malvado-sdk-info.txt")
if(NOT EXISTS "${sdk_info}")
  message(FATAL_ERROR "SDK information is missing")
endif()
file(READ "${sdk_info}" sdk_info_content)
if(NOT sdk_info_content STREQUAL "version=${sdk_version}\nsystem=${sdk_system}\narchitecture=${sdk_arch}\n")
  message(FATAL_ERROR "SDK information does not match the build")
endif()
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
  if(HAS_GRAPHICS)
    execute_process(COMMAND "${WORK}/moved sdk/${BINDIR}/${CLI_NAME}" new --c "${WORK}/C game"
      RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
      message(FATAL_ERROR "C project creation failed")
    endif()
    if(RUN_GRAPHICS)
      file(WRITE "${WORK}/C game/smoke.c" [=[
#include <malvado/platform.h>
static int should_close(void) { static int frames; return ++frames > 3; }
#define mv_window_should_close should_close
#define main project_main
#include "main.c"
#undef main
int main(void) { return project_main(); }
]=])
      file(APPEND "${WORK}/C game/CMakeLists.txt" [=[
enable_testing()
add_executable(smoke smoke.c)
target_link_libraries(smoke PRIVATE Malvado::graphics)
add_test(NAME smoke COMMAND smoke)
]=])
    endif()
    execute_process(COMMAND "${CMAKE_COMMAND}" -S "${WORK}/C game" -B "${WORK}/c-build"
      "-DCMAKE_PREFIX_PATH=${WORK}/moved sdk" "-DCMAKE_C_COMPILER=${C_COMPILER}"
      "-DCMAKE_C_FLAGS=${C_FLAGS}" "-DCMAKE_EXE_LINKER_FLAGS=${EXE_LINKER_FLAGS}"
      RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
      message(FATAL_ERROR "C project configuration failed")
    endif()
    execute_process(COMMAND "${CMAKE_COMMAND}" --build "${WORK}/c-build" --config "${CONFIG}"
      RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
      message(FATAL_ERROR "C project build failed")
    endif()
    if(RUN_GRAPHICS)
      execute_process(COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${WORK}/c-build" -C "${CONFIG}" --output-on-failure
        RESULT_VARIABLE result)
      if(NOT result EQUAL 0)
        message(FATAL_ERROR "C project graphics check failed")
      endif()
    endif()
  endif()
  if(CAN_EXPORT)
    set(installed_cli "${WORK}/moved sdk/${BINDIR}/${CLI_NAME}")
    set(installed_launcher "${WORK}/moved sdk/${LIBEXECDIR}/malvado/${LAUNCHER_NAME}")
    execute_process(COMMAND "${installed_cli}" doctor RESULT_VARIABLE result OUTPUT_VARIABLE output)
    if(NOT result EQUAL 0 OR NOT output MATCHES "Export: launchers and required licenses found")
      message(FATAL_ERROR "Doctor did not detect the installed SDK: ${output}")
    endif()
    file(RENAME "${installed_launcher}" "${installed_launcher}.saved")
    execute_process(COMMAND "${installed_cli}" doctor RESULT_VARIABLE result OUTPUT_VARIABLE output)
    file(RENAME "${installed_launcher}.saved" "${installed_launcher}")
    if(NOT result EQUAL 0 OR NOT output MATCHES "Export: launchers or required licenses missing")
      message(FATAL_ERROR "Doctor did not detect a missing launcher: ${output}")
    endif()
    set(native_launcher "${WORK}/moved sdk/${LIBEXECDIR}/malvado/malvado-native-game")
    file(RENAME "${native_launcher}" "${native_launcher}.saved")
    execute_process(COMMAND "${installed_cli}" doctor RESULT_VARIABLE result OUTPUT_VARIABLE output)
    file(RENAME "${native_launcher}.saved" "${native_launcher}")
    if(NOT result EQUAL 0 OR NOT output MATCHES "Export: launchers or required licenses missing")
      message(FATAL_ERROR "Doctor did not detect a missing native launcher: ${output}")
    endif()
    file(RENAME "${notices}/raylib-LICENSE" "${notices}/raylib-LICENSE.saved")
    execute_process(COMMAND "${installed_cli}" doctor RESULT_VARIABLE result OUTPUT_VARIABLE output)
    file(MAKE_DIRECTORY "${WORK}/missing-license-project")
    file(WRITE "${WORK}/missing-license-project/main.lua" "print('game')\n")
    execute_process(COMMAND "${installed_cli}" export "${WORK}/missing-license-export"
      WORKING_DIRECTORY "${WORK}/missing-license-project" RESULT_VARIABLE export_result)
    file(RENAME "${notices}/raylib-LICENSE.saved" "${notices}/raylib-LICENSE")
    if(NOT result EQUAL 0 OR NOT output MATCHES "Export: launchers or required licenses missing" OR
       export_result EQUAL 0 OR EXISTS "${WORK}/missing-license-export")
      message(FATAL_ERROR "Doctor did not detect a missing license: ${output}")
    endif()
    if(HAS_ASSETS)
      file(RENAME "${notices}/cJSON-LICENSE" "${notices}/cJSON-LICENSE.saved")
      execute_process(COMMAND "${installed_cli}" doctor RESULT_VARIABLE result OUTPUT_VARIABLE output)
      file(RENAME "${notices}/cJSON-LICENSE.saved" "${notices}/cJSON-LICENSE")
      if(NOT result EQUAL 0 OR NOT output MATCHES "Export: launchers or required licenses missing")
        message(FATAL_ERROR "Doctor did not detect a missing cJSON license: ${output}")
      endif()
    endif()
    execute_process(COMMAND "${CMAKE_COMMAND}"
      "-DCLI=${WORK}/moved sdk/${BINDIR}/${CLI_NAME}"
      "-DWORK=${WORK}/export" "-DVISUAL=${RUN_GRAPHICS}"
      -P "${SOURCE}/tests/export_game.cmake" RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
      message(FATAL_ERROR "Game export test failed")
    endif()
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DCLI=${installed_cli}"
      "-DWORK=${WORK}/native-cli" "-DC_FLAGS=${C_FLAGS}" "-DEXE_LINKER_FLAGS=${EXE_LINKER_FLAGS}"
      -P "${SOURCE}/tests/native_cli.cmake" RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
      message(FATAL_ERROR "Native client test failed")
    endif()
  endif()
endif()
configure_file("${SOURCE}/examples/c/processes/main.c" "${WORK}/game/main.c" COPYONLY)
configure_file("${SOURCE}/tests/animation.c" "${WORK}/game/animation.c" COPYONLY)
configure_file("${SOURCE}/tests/camera.c" "${WORK}/game/camera.c" COPYONLY)
configure_file("${SOURCE}/examples/c/control/main.c" "${WORK}/game/control.c" COPYONLY)
configure_file("${SOURCE}/examples/c/inspection/main.c" "${WORK}/game/inspection.c" COPYONLY)
file(WRITE "${WORK}/game/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.20)
project(sdk_game C)
find_package(Malvado CONFIG REQUIRED COMPONENTS core)
add_executable(game main.c)
target_link_libraries(game PRIVATE Malvado::core)
enable_testing()
add_test(NAME game COMMAND game)
add_executable(animation_example animation.c)
target_link_libraries(animation_example PRIVATE Malvado::core)
add_test(NAME animation_example COMMAND animation_example)
add_executable(camera_example camera.c)
target_link_libraries(camera_example PRIVATE Malvado::core)
add_test(NAME camera_example COMMAND camera_example)
add_executable(control_example control.c)
target_link_libraries(control_example PRIVATE Malvado::core)
add_test(NAME control_example COMMAND control_example)
add_executable(inspection_example inspection.c)
target_link_libraries(inspection_example PRIVATE Malvado::core)
add_test(NAME inspection_example COMMAND inspection_example)
]=])
if(HAS_ASSETS)
  configure_file("${SOURCE}/tests/fpg.c" "${WORK}/game/fpg.c" COPYONLY)
  configure_file("${SOURCE}/tests/fixtures/fpg/bennugd-8.fpg" "${WORK}/game/bennugd-8.fpg" COPYONLY)
  configure_file("${SOURCE}/tests/atlas.c" "${WORK}/game/atlas.c" COPYONLY)
  configure_file("${SOURCE}/examples/c/atlas/main.c" "${WORK}/game/atlas-example.c" COPYONLY)
  configure_file("${SOURCE}/tests/fixtures/atlas.json" "${WORK}/game/atlas.json" COPYONLY)
  configure_file("${SOURCE}/tests/fixtures/animated-atlas.json" "${WORK}/game/animated-atlas.json" COPYONLY)
  configure_file("${SOURCE}/tests/fixtures/aseprite/array.json" "${WORK}/game/aseprite.json" COPYONLY)
  configure_file("${SOURCE}/tests/fixtures/aseprite/trimmed.json" "${WORK}/game/trimmed.json" COPYONLY)
  configure_file("${SOURCE}/tests/fixtures/aseprite/pingpong.json" "${WORK}/game/pingpong.json" COPYONLY)
  configure_file("${SOURCE}/tests/fixtures/aseprite/pivot.json" "${WORK}/game/pivot.json" COPYONLY)
  configure_file("${SOURCE}/tests/fixtures/aseprite/coric/coric.json" "${WORK}/game/coric.json" COPYONLY)
  file(APPEND "${WORK}/game/CMakeLists.txt" [=[
find_package(Malvado CONFIG REQUIRED COMPONENTS assets)
add_executable(fpg_test fpg.c)
target_link_libraries(fpg_test PRIVATE Malvado::assets)
add_test(NAME fpg COMMAND fpg_test "${CMAKE_CURRENT_SOURCE_DIR}/bennugd-8.fpg")
add_executable(atlas_example atlas-example.c)
target_link_libraries(atlas_example PRIVATE Malvado::assets)
add_test(NAME atlas_example COMMAND atlas_example)
add_executable(atlas_test atlas.c)
target_link_libraries(atlas_test PRIVATE Malvado::assets)
add_test(NAME atlas COMMAND atlas_test "${CMAKE_CURRENT_SOURCE_DIR}/atlas.json" "${CMAKE_CURRENT_SOURCE_DIR}/animated-atlas.json" "${CMAKE_CURRENT_SOURCE_DIR}/aseprite.json" "${CMAKE_CURRENT_SOURCE_DIR}/trimmed.json" "${CMAKE_CURRENT_SOURCE_DIR}/pingpong.json" "${CMAKE_CURRENT_SOURCE_DIR}/pivot.json" "${CMAKE_CURRENT_SOURCE_DIR}/coric.json")
]=])
endif()
if(HAS_GRAPHICS)
  file(WRITE "${WORK}/game/graphics.c" [=[
#include <limits.h>
#include "malvado/platform.h"
int main(void) {
  if (!mv_window_open(160, 100, "Installed SDK")) return 1;
  int width = mv_window_text_width("Hello", 20);
  int overflow = mv_window_text_width("MMMMMMMMMMMMMMMM", INT_MAX);
  if (width <= 0 || overflow != 0) {
    mv_window_close();
    return 1;
  }
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
