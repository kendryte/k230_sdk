set(CMAKE_BUILD_TYPE "Static")
add_executable(peripheral peripheral.c)
target_link_libraries(peripheral Binc)