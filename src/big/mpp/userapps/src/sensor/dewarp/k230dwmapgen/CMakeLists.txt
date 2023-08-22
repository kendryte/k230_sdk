cmake_minimum_required(VERSION 3.13)
project (k230dwmapgen)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)

include_directories(include)
include_directories(${LIBYAML_INCLUDE_DIRS})
add_library(${PROJECT_NAME} src/basic.cpp src/DewarpMap.cpp)

add_executable(${PROJECT_NAME}-dump exe/main.c)
target_link_libraries(${PROJECT_NAME}-dump ${PROJECT_NAME} yaml)
link_directories(${LIBYAML_LIBRARY_DIRS})

add_executable(${PROJECT_NAME}-splitscreen exe/split_screen.c)
target_link_libraries(${PROJECT_NAME}-splitscreen ${PROJECT_NAME})
