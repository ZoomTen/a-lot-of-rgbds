#
# This file is part of RGBDS.
#
# Copyright (c) 2020 RGBDS contributors.
#
# SPDX-License-Identifier: MIT
#

# 3.9 required for LTO checks
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

project(rgbds
        LANGUAGES C)

# get real path of source and binary directories
get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

# reject in-source builds, may conflict with Makefile
if(srcdir STREQUAL bindir)
  message("RGBDS should not be built in the source directory.")
  message("Instead, create a separate build directory and specify to CMake the path to the source directory.")
  message(FATAL_ERROR "Terminating configuration")
endif()

option(SANITIZERS "Build with sanitizers enabled" OFF) # Ignored on MSVC
option(MORE_WARNINGS "Turn on more warnings" OFF) # Ignored on MSVC

if(MSVC)
  # MSVC's standard library triggers warning C5105,
  # "macro expansion producing 'defined' has undefined behavior"
  add_compile_options(/std:c11 /W1 /MP /wd5105)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -pedantic)
  add_definitions(-D_POSIX_C_SOURCE=200809L -D_ISOC11_SOURCE)
  if(SANITIZERS)
    set(SAN_FLAGS -fsanitize=shift -fsanitize=integer-divide-by-zero
                  -fsanitize=unreachable -fsanitize=vla-bound
                  -fsanitize=signed-integer-overflow -fsanitize=bounds
                  -fsanitize=object-size -fsanitize=bool -fsanitize=enum
                  -fsanitize=alignment -fsanitize=null -fsanitize=address)
    add_compile_options(${SAN_FLAGS})
    link_libraries(${SAN_FLAGS})
    # A non-zero optimization level is desired in debug mode, but allow overriding it nonetheless
    # TODO: this overrides anything previously set... that's a bit sloppy!
    set(CMAKE_C_FLAGS_DEBUG "-g -Og -fno-omit-frame-pointer -fno-optimize-sibling-calls" CACHE STRING "" FORCE)
  endif()

  if(MORE_WARNINGS)
    add_compile_options(-Werror -Wextra
                        -Walloc-zero -Wcast-align -Wcast-qual -Wduplicated-branches -Wduplicated-cond
                        -Wfloat-equal -Winline -Wlogical-op -Wnested-externs -Wnull-dereference
                        -Wold-style-definition -Wshift-overflow=2 -Wstrict-overflow=5
                        -Wstrict-prototypes -Wstringop-overflow=4 -Wundef -Wuninitialized -Wunused
                        -Wshadow # TODO: -Wshadow=compatible-local ?
                        -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=1
                        -Wno-format-nonliteral # We have a couple of "dynamic" prints
                        # We do some range checks that are always false on some platforms (GCC, Clang)
                        -Wno-type-limits -Wno-tautological-constant-out-of-range-compare
                        -Wvla # MSVC does not support VLAs
                        -Wno-unknown-warning-option) # Clang shouldn't diagnose unknown warnings
  endif()
endif()

# Use versioning consistent with Makefile
# the git revision is used but uses the fallback in an archive

find_program(GIT git)
if(GIT)
  execute_process(COMMAND ${GIT} describe --tags --dirty --always
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE GIT_REV OUTPUT_STRIP_TRAILING_WHITESPACE
                  ERROR_QUIET)
  message(STATUS "RGBDS version: ${GIT_REV}")
else(GIT)
  message(STATUS "Cannot determine RGBDS version (Git not installed), falling back")
endif(GIT)

include_directories("${PROJECT_SOURCE_DIR}/include")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_subdirectory(src)

# By default, build in Release mode; Debug mode must be explicitly requested
# (You may want to augment it with the options above)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(CHECK_START "Checking if LTO is supported")
  include(CheckIPOSupported)
  check_ipo_supported(RESULT enable_lto)
  if(enable_lto)
    message(CHECK_PASS "yes")
    set_property(TARGET rgbasm rgblink rgbfix rgbgfx PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
  else()
    message(CHECK_FAIL "no")
  endif()
endif()
