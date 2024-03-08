# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nano_service_arch)

list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)

set(NANOPB_OPTIONS "--c-style")
zephyr_nanopb_sources(app include/services/indicator.proto)
zephyr_nanopb_sources(app include/services/trigger.proto)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/include/services)

file(GLOB app_sources include/services/*.c)

if(CONFIG_BUTTON_TRIGGER)
  list(APPEND app_sources src/button_trigger.c)
elseif(CONFIG_TIMER_TRIGGER)
  list(APPEND app_sources src/timer_trigger.c)
endif()

target_sources(app PRIVATE ${app_sources} src/main.c src/led_indicator.c)
