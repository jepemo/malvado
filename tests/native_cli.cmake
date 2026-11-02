# Malvado 2; project created by Jeremías Pérez and contributors.
# SPDX-License-Identifier: Apache-2.0

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/C game/assets" "${WORK}/elsewhere")
file(WRITE "${WORK}/C game/assets/value.txt" "asset")
file(WRITE "${WORK}/C game/main.c" [=[
#include <stdio.h>
#include <string.h>
#include <malvado/malvado.h>
int main(void) {
  char text[6] = {0};
  FILE *file = fopen("assets/value.txt", "rb");
  if (!file) return 1;
  size_t n = fread(text, 1, 5, file);
  fclose(file);
  if (n != 5 || strcmp(text, "asset") != 0) return 1;
  MvContext *ctx = mv_context_create(60, 5);
  if (!ctx) return 1;
  mv_context_destroy(ctx);
  puts("native game works");
  return 0;
}
]=])
file(WRITE "${WORK}/C game/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.20)
project(game C)
find_package(Malvado CONFIG REQUIRED COMPONENTS core)
add_executable(game main.c)
target_link_libraries(game PRIVATE Malvado::core)
]=])
execute_process(COMMAND "${CMAKE_COMMAND}" -E env "CFLAGS=${C_FLAGS}" "LDFLAGS=${EXE_LINKER_FLAGS}"
  "${CLI}" build "${WORK}/C game" WORKING_DIRECTORY "${WORK}/elsewhere" RESULT_VARIABLE result)
if(NOT result EQUAL 0 OR NOT EXISTS "${WORK}/C game/.malvado/build/game")
  message(FATAL_ERROR "Native build failed")
endif()
execute_process(COMMAND "${CLI}" run "${WORK}/C game" WORKING_DIRECTORY "${WORK}/elsewhere"
  RESULT_VARIABLE result OUTPUT_VARIABLE output)
if(NOT result EQUAL 0 OR NOT output MATCHES "native game works")
  message(FATAL_ERROR "Native run failed: ${output}")
endif()
execute_process(COMMAND "${CLI}" export WORKING_DIRECTORY "${WORK}/C game" RESULT_VARIABLE result)
if(NOT result EQUAL 0 OR NOT EXISTS "${WORK}/C game/dist/game/game-bin" OR
   EXISTS "${WORK}/C game/dist/game/main.c" OR EXISTS "${WORK}/C game/dist/game/CMakeLists.txt")
  message(FATAL_ERROR "Native export failed or included source files")
endif()
execute_process(COMMAND "${WORK}/C game/dist/game/game" WORKING_DIRECTORY "${WORK}/elsewhere"
  RESULT_VARIABLE result OUTPUT_VARIABLE output)
if(NOT result EQUAL 0 OR NOT output MATCHES "native game works")
  message(FATAL_ERROR "Exported native game failed: ${output}")
endif()
execute_process(COMMAND "${CLI}" run --headless "${WORK}/C game" WORKING_DIRECTORY "${WORK}/elsewhere"
  RESULT_VARIABLE result)
if(NOT result EQUAL 2)
  message(FATAL_ERROR "Native headless mode must report invalid usage")
endif()
file(MAKE_DIRECTORY "${WORK}/other/.malvado/build")
file(WRITE "${WORK}/other/main.c" "int main(void) { return 0; }\n")
file(WRITE "${WORK}/other/CMakeLists.txt" "project(other C)\n")
file(WRITE "${WORK}/other/.malvado/build/user-file" "keep")
execute_process(COMMAND "${CLI}" build "${WORK}/other" RESULT_VARIABLE result)
file(READ "${WORK}/other/.malvado/build/user-file" content)
if(result EQUAL 0 OR NOT content STREQUAL "keep")
  message(FATAL_ERROR "An unmanaged build directory was not protected")
endif()
file(REMOVE_RECURSE "${WORK}")
