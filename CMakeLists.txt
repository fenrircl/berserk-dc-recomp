cmake_minimum_required(VERSION 3.24)
project(berserk_recomp VERSION 0.1.0 LANGUAGES C CXX)

# Sword of the Berserk: Guts' Rage as a native PC port, built with dreamcomp.
# The port normally lives at <dreamcomp>/ports/berserk-recomp, so dreamcomp is two levels up.
# An external checkout can be used with -DDREAMCOMP_DIR=<path> (dc.py and build_linux.sh pass it).
set(DREAMCOMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "dreamcomp checkout")
if(NOT EXISTS "${DREAMCOMP_DIR}/cmake/DreamcompPort.cmake")
  set(DREAMCOMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dreamcomp" CACHE PATH "dreamcomp checkout" FORCE)
endif()
if(NOT EXISTS "${DREAMCOMP_DIR}/cmake/DreamcompPort.cmake")
  message(FATAL_ERROR "dreamcomp not found at ${DREAMCOMP_DIR}. Clone it and place this repo at "
                      "<dreamcomp>/ports/berserk-recomp, or pass -DDREAMCOMP_DIR=<path>.")
endif()

add_subdirectory(${DREAMCOMP_DIR} dreamcomp)
add_subdirectory(game)
