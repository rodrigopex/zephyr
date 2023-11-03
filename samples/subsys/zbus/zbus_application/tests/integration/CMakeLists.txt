# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(calculator)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_sources(
  app
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../src/calculator_scope.c"
          "${CMAKE_CURRENT_SOURCE_DIR}/../../src/calculator_scope_impl.c"
          "${CMAKE_CURRENT_SOURCE_DIR}/../../src/io.c"
          "${CMAKE_CURRENT_SOURCE_DIR}/../../src/net_scope.c"
          "${CMAKE_CURRENT_SOURCE_DIR}/../../src/net_scope_impl.c")
