# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(spiram_rodata_memtest)

# zephyr_sources_ifdef(CONFIG_MEMTEST_INLINE src/spiram_memtest.c)
# zephyr_sources_ifdef(CONFIG_MEMTEST_BSS src/spiram_memtest_bss.c)
# zephyr_sources_ifdef(CONFIG_MEMTEST_HEAP src/spiram_memtest_heap.c)

target_sources(app PRIVATE src/main.c)
