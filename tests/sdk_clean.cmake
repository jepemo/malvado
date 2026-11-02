# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

if(NOT ARCHIVE OR NOT SOURCE OR NOT WORK)
  message(FATAL_ERROR "Provide ARCHIVE, SOURCE, and WORK")
endif()
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/sdk" "${WORK}/outside")
if(NOT EXISTS "${ARCHIVE}.sha256")
  message(FATAL_ERROR "SDK checksum is missing")
endif()
file(SHA256 "${ARCHIVE}" actual)
file(READ "${ARCHIVE}.sha256" recorded)
string(FIND "${recorded}" "${actual}" position)
if(NOT position EQUAL 0)
  message(FATAL_ERROR "SDK checksum does not match")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xzf "${ARCHIVE}" WORKING_DIRECTORY "${WORK}/sdk"
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Cannot extract SDK")
endif()
set(cli "${WORK}/sdk/bin/malvado")
foreach(test cli_project native_cli export_game)
  execute_process(COMMAND "${CMAKE_COMMAND}" "-DCLI=${cli}" "-DWORK=${WORK}/${test}"
    "-DVISUAL=${VISUAL}" -P "${SOURCE}/tests/${test}.cmake" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Installed SDK ${test} check failed")
  endif()
endforeach()
if(VISUAL)
  set(project "${WORK}/C graphical game")
  execute_process(COMMAND "${cli}" new --c "${project}" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "C game creation failed")
  endif()
  file(WRITE "${project}/data.txt" "asset")
  file(WRITE "${project}/main.c" [=[
#include <stdio.h>
#include <malvado/platform.h>
int main(void) {
  FILE *file = fopen("data.txt", "rb");
  if (!file) return 1;
  fclose(file);
  if (!mv_window_open(160, 100, "C game")) return 1;
  for (int i = 0; i < 3; ++i) {
    mv_window_begin((MvColor){0, 0, 0, 255});
    mv_window_draw_text("Hello", 10, 10, 20, (MvColor){255, 255, 255, 255});
    mv_window_end();
  }
  mv_window_close();
  return 0;
}
]=])
  execute_process(COMMAND "${cli}" run "${project}" WORKING_DIRECTORY "${WORK}/outside" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "C game run failed")
  endif()
  execute_process(COMMAND "${cli}" export WORKING_DIRECTORY "${project}" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "C game export failed")
  endif()
  execute_process(COMMAND "${project}/dist/game/game" WORKING_DIRECTORY "${WORK}/outside" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Exported C game failed")
  endif()
endif()
file(REMOVE_RECURSE "${WORK}")
