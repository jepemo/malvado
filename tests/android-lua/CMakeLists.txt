# Malvado 2; SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20)
project(android_lua C)
set(MALVADO_BUILD_LUA ON CACHE BOOL "" FORCE)
set(MALVADO_BUILD_ASSETS ON CACHE BOOL "" FORCE)
add_subdirectory(../.. engine EXCLUDE_FROM_ALL)
add_library(game SHARED main.c)
target_include_directories(game PRIVATE ../../src "${ANDROID_NDK}/sources/android/native_app_glue")
target_link_libraries(game PRIVATE malvado_lua_binding)
target_link_options(game PRIVATE "-uANativeActivity_onCreate" "-Wl,--no-undefined")
target_compile_definitions(game PRIVATE ACCEPTANCE_TOKEN="${ACCEPTANCE_TOKEN}" ACCEPTANCE_FAILURE=${ACCEPTANCE_FAILURE})

target_compile_definitions(game PRIVATE ACCEPTANCE_LIFECYCLE=${ACCEPTANCE_LIFECYCLE})
target_include_directories(game PRIVATE "${ANDROID_NDK}/sources/android/native_app_glue")
target_include_directories(game PRIVATE ../android-window)

target_compile_definitions(game PRIVATE ACCEPTANCE_AUDIO=${ACCEPTANCE_AUDIO})
