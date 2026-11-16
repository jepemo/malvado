# Malvado 2; SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20)
project(android_sdk_consumer C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_FIND_USE_PACKAGE_REGISTRY OFF)
set(CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY OFF)
# This explicit prefix contains the target SDK, outside the NDK sysroot.
find_package(Malvado CONFIG REQUIRED COMPONENTS core assets
  PATHS "${MALVADO_SDK_PREFIX}" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
add_executable(android-sdk-consumer main.c)
target_link_libraries(android-sdk-consumer PRIVATE Malvado::core Malvado::assets)
target_compile_definitions(android-sdk-consumer PRIVATE EXPECTED_VERSION="${EXPECTED_VERSION}")
