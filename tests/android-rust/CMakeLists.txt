# Malvado 2; SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20)
project(android_rust_window C)
find_package(Malvado CONFIG REQUIRED COMPONENTS graphics
  PATHS "${MALVADO_TEST_SDK}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
add_library(game SHARED main.c)
target_link_libraries(game PRIVATE "${RUST_ARCHIVE}" Malvado::graphics)
target_link_options(game PRIVATE "-Wl,--no-undefined" "-uANativeActivity_onCreate")
target_compile_definitions(game PRIVATE ACCEPTANCE_TOKEN="${ACCEPTANCE_TOKEN}")
