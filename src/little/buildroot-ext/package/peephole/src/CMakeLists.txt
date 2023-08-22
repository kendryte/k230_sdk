cmake_minimum_required(VERSION 3.10)
project(peephole C CXX)

set(CMAKE_INSTALL_PREFIX "/app/peephole")
set(CMAKE_INSTALL_LIBDIR "/usr/lib")
set(CMAKE_INSTALL_BINDIR "/usr/bin")

option(DEMO_BUILD "Build demo code" OFF)

add_subdirectory(ui)

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
