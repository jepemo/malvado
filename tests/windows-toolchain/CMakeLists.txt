# Malvado 2; SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.31)
project(windows_toolchain_probe C)
set(CMAKE_C_STANDARD 11)
file(TO_CMAKE_PATH "${CMAKE_C_COMPILER}" compiler)
file(TO_CMAKE_PATH "${CMAKE_GENERATOR_INSTANCE}" instance)
file(WRITE "${CMAKE_BINARY_DIR}/selection.txt"
  "generator=${CMAKE_GENERATOR}\nplatform=${CMAKE_GENERATOR_PLATFORM}\ntoolset=${CMAKE_GENERATOR_TOOLSET}\ninstance=${instance}\nvs_build=${CMAKE_VS_VERSION_BUILD_NUMBER}\ncompiler=${compiler}\ncompiler_id=${CMAKE_C_COMPILER_ID}\ncompiler_version=${CMAKE_C_COMPILER_VERSION}\nsdk=${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}\n")
add_executable(toolchain-probe main.c)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/program-$<CONFIG>.txt" CONTENT "$<TARGET_FILE:toolchain-probe>")
