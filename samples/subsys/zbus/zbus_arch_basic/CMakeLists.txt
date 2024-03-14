# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nano_service_arch)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/include/services)

include(nano_services.cmake)
nano_service_cre_register(app include/services/indicator.proto)

nano_service_e_register(app include/services/trigger.proto)

if(CONFIG_BUTTON_TRIGGER)
  target_sources(app PRIVATE src/button_trigger.c)
elseif(CONFIG_TIMER_TRIGGER)
  target_sources(app PRIVATE src/timer_trigger.c)
endif()

target_sources(app PRIVATE src/main.c src/led_indicator.c)
