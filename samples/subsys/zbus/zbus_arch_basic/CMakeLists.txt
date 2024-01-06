# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(button)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB app_sources include/ifaces/*.c)

if(CONFIG_BUTTON_TRIGGER)
  list(APPEND app_sources src/button_trigger.c)
elseif(CONFIG_TIMER_TRIGGER)
  list(APPEND app_sources src/timer_trigger.c)
endif()

target_sources(app PRIVATE ${app_sources} src/main.c src/led_indicator.c)
